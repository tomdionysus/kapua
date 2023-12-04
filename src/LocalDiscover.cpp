//
// Kapua LocalDiscovery class
//
// Author: Tom Cully <mail@tomcully.com>
//
#include "LocalDiscover.hpp"

namespace Kapua {

LocalDiscover::LocalDiscover(Logger* logger) : _socket_fd(-1) { _logger = new ScopedLogger("LocalDiscover", logger); }

LocalDiscover::~LocalDiscover() {
  stop();
  delete _logger;
}

bool LocalDiscover::start(int port) {
  std::lock_guard<std::mutex> lock(_running_mutex);

  if (_running) {
    _logger->warn("start called, but thread already running");
    return false;
  }

  _main_thread = new std::thread(&LocalDiscover::_main_loop, this);
  return true;
}

bool LocalDiscover::stop() {
  std::lock_guard<std::mutex> lock(_running_mutex);

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
    _logger->error("Failed to create socke");
    return false;
  }

  // Set up server address
  _server_addr.sin_family = AF_INET;
  _server_addr.sin_port = htons(port);
  _server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  // Set the recieve timeout
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 100000;
  if (setsockopt(_socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
    _logger->error("Failed to set socket options");
  }

  // Bind the socket
  if (bind(_socket_fd, (struct sockaddr*)&_server_addr, sizeof(_server_addr)) == -1) {
    _logger->error("Failed to bind socket");
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
  bool running = true;
  sockaddr_in from_addr;

  while (running) {
    ssize_t len = _receive((char*)(&buffer), 65536, from_addr);

    if (len > -1) {
      _logger->debug("Packet From " + std::string(inet_ntoa(from_addr.sin_addr)) + ":" +std::to_string(ntohs(from_addr.sin_port)) + ", " +
                    std::to_string(len) + " bytes.");
    }

    // Stop if flagged
    _running_mutex.lock();
    running = _running;
    _running_mutex.unlock();
  }
}

ssize_t LocalDiscover::_receive(char* buffer, size_t buffer_size, sockaddr_in& client_addr) {
  socklen_t client_len = sizeof(client_addr);
  return recvfrom(_socket_fd, buffer, buffer_size, 0, (struct sockaddr*)&client_addr, &client_len);
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
