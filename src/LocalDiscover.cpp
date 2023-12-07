//
// Kapua LocalDiscover class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023 
//
#include "LocalDiscover.hpp"

#include "Protocol.hpp"

namespace Kapua {

LocalDiscover::LocalDiscover(Logger* logger) {
  _logger = new ScopedLogger("LocalDiscover", logger);
  _running = false;
}

LocalDiscover::~LocalDiscover() {
  if (_running) stop();
  delete _logger;
}

bool LocalDiscover::start(int port) {
  _logger->debug("Starting...");
  if (_running) {
    _logger->warn("start called, but thread already running");
    return false;
  }

  _main_thread = new std::thread(&LocalDiscover::_main_loop, this);
  return true;
}

bool LocalDiscover::stop() {
  if (!_running) {
    _logger->warn("stop called, but thread not running");
    return false;
  }
  _running = false;

  _main_thread->join();

  delete _main_thread;
  _main_thread = nullptr;

  return true;
}

bool LocalDiscover::_listen(int port) {
  // Create a server socket
  _server_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (_server_socket_fd == -1) {
    _logger->error("Failed creating server socket");
    return false;
  }

  // Set up server address
  _server_addr.sin_family = AF_INET;
  _server_addr.sin_port = htons(port);
  _server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  // Bind the server socket
  if (bind(_server_socket_fd, (struct sockaddr*)&_server_addr, sizeof(_server_addr)) == -1) {
    _logger->error("Failed binding server socket");
    _shutdown();
    return false;
  }

  // Create a sending socket
  _client_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (_client_socket_fd == -1) {
    _logger->error("Failed creating send socket");
    return false;
  }

  int broadcastEnable = 1;
  if (setsockopt(_client_socket_fd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) == -1) {
    _logger->error("Failed setting send socket options");
    _shutdown();
    return false;
  }

#ifdef _WIN32
  // initialize Windows Socket API (Winsock)
  if (WSAStartup(MAKEWORD(2, 2), &_wsaData) != 0) {
    _logger->error("WIN32: Failed to initialize Winsock");
    _shutdown();
    return false;
  }
#endif

  return true;
}

void LocalDiscover::_main_loop() {
  char buffer[65536];
  sockaddr_in from_addr;

  _running = true;

  if (!_listen(KAPUA_PORT)) {
    _logger->error("Listen failed");
    _shutdown();
    return;
  }

  auto last_time_called = std::chrono::steady_clock::now() - std::chrono::hours(24);

  while (_running) {
    // Do a recieve
    ssize_t len = _receive((char*)(&buffer), 65536, from_addr);
    if (len > -1) {
      _logger->debug("Packet From " + std::string(inet_ntoa(from_addr.sin_addr)) + ":" + std::to_string(ntohs(from_addr.sin_port)) + ", " +
                     std::to_string(len) + " bytes.");
    }

    // Do a send if time
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - last_time_called);

    if (duration.count() >= 5) {
      _broadcast();

      // Reset the last_time_called variable
      last_time_called = now;
    }
  }

  _logger->debug("Stopping...");
  _shutdown();

  _logger->debug("Stopped");
}

void LocalDiscover::_broadcast() {
  Packet* pkt = static_cast<Packet*>(calloc(512, 1));

  pkt->from_id = 0xFFFFFFFF12345678;
  pkt->to_id = KAPUA_ID_BROADCAST;
  pkt->length = 0;

  struct sockaddr_in broadcast_addr;
  std::memset(&broadcast_addr, 0, sizeof(broadcast_addr));
  broadcast_addr.sin_family = AF_INET;
  broadcast_addr.sin_port = htons(KAPUA_PORT);  // Specify the port you want to use for broadcasting
  broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

  _logger->debug("Discovery broadcast");
  ssize_t res = _send((char*)pkt, sizeof(Packet), broadcast_addr);
  if (res == -1) {
    _logger->error("Discovery broadcast error " + std::string(strerror(errno)));
  }

  free(pkt);
}

ssize_t LocalDiscover::_receive(char* buffer, size_t buffer_size, sockaddr_in& client_addr) {
  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 0;

  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(_server_socket_fd, &rfds);
  int recVal = select(_server_socket_fd + 1, &rfds, NULL, NULL, &tv);

  if (recVal > 0) {
    socklen_t client_len = sizeof(client_addr);
    return recvfrom(_server_socket_fd, buffer, buffer_size, 0, (struct sockaddr*)&client_addr, &client_len);
  } else {
    return -1;
  }
}

ssize_t LocalDiscover::_send(const char* buffer, size_t len, const sockaddr_in& client_addr) {
  return sendto(_client_socket_fd, buffer, len, 0, (const struct sockaddr*)&client_addr, sizeof(client_addr));
}

bool LocalDiscover::_shutdown() {
  if (_server_socket_fd != -1) {
#ifdef _WIN32
    closesocket(_server_socket_fd);
#else
    close(_server_socket_fd);
#endif
    _server_socket_fd = -1;
  }

  if (_client_socket_fd != -1) {
#ifdef _WIN32
    closesocket(_client_socket_fd);
#else
    close(_client_socket_fd);
#endif
    _client_socket_fd = -1;
  }

#ifdef _WIN32
  WSACleanup();
#endif

  return true;
}

}  // namespace Kapua
