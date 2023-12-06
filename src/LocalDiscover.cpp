//
// Kapua LocalDiscover class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023 
//
#include "LocalDiscover.hpp"

namespace Kapua {

LocalDiscover::LocalDiscover(Logger* logger) : _socket_fd(-1) {
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
  // Create a socket
  _socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (_socket_fd == -1) {
    _logger->error("Failed creating socket");
    return false;
  }

  // Set up server address
  _server_addr.sin_family = AF_INET;
  _server_addr.sin_port = htons(port);
  _server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  // Bind the socket
  if (bind(_socket_fd, (struct sockaddr*)&_server_addr, sizeof(_server_addr)) == -1) {
    _logger->error("Failed binding socket");
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

  while (_running) {
    ssize_t len = _receive((char*)(&buffer), 65536, from_addr);

    if (len > -1) {
      _logger->debug("Packet From " + std::string(inet_ntoa(from_addr.sin_addr)) + ":" + std::to_string(ntohs(from_addr.sin_port)) + ", " +
                     std::to_string(len) + " bytes.");
    }
  }

  _logger->debug("Stopping...");
  _shutdown();

  _logger->info("Stopped");
}

ssize_t LocalDiscover::_receive(char* buffer, size_t buffer_size, sockaddr_in& client_addr) {
  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 0;

  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(_socket_fd, &rfds);
  int recVal = select(_socket_fd + 1, &rfds, NULL, NULL, &tv);

  if (recVal > 0) {
    socklen_t client_len = sizeof(client_addr);
    return recvfrom(_socket_fd, buffer, buffer_size, 0, (struct sockaddr*)&client_addr, &client_len);
  } else {
    return -1;
  }
}

ssize_t LocalDiscover::_send(const char* buffer, size_t len, const sockaddr_in& client_addr) {
  return sendto(_socket_fd, buffer, len, 0, (const struct sockaddr*)&client_addr, sizeof(client_addr));
}

bool LocalDiscover::_shutdown() {
  if (_socket_fd != -1) {
#ifdef _WIN32
    closesocket(_socket_fd);
    WSACleanup();
#else
    close(_socket_fd);
#endif
    _socket_fd = -1;
  }

  return true;
}

}  // namespace Kapua
