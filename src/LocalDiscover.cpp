//
// Kapua LocalDiscovery class
//
// Author: Tom Cully <mail@tomcully.com>
//
#include "LocalDiscover.hpp"

namespace Kapua {

LocalDiscover::LocalDiscover(Logger* logger) : _socket_fd(-1) { _logger = new ScopedLogger("LocalDiscover", logger); }

LocalDiscover::~LocalDiscover() {
  shutdown();
  delete _logger;
}

bool LocalDiscover::start(int port) {
  // Create a socket
  _socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (_socket_fd == -1) {
    std::cerr << "Failed to create socket" << std::endl;
    return false;
  }

  // Set up server address
  _server_addr.sin_family = AF_INET;
  _server_addr.sin_port = htons(port);
  _server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  // Bind the socket
  if (bind(_socket_fd, (struct sockaddr*)&_server_addr, sizeof(_server_addr)) == -1) {
    std::cerr << "Failed to bind socket" << std::endl;
    shutdown();
    return false;
  }

#ifdef _WIN32
  // initialize Windows Socket API (Winsock)
  if (WSAStartup(MAKEWORD(2, 2), &_wsaData) != 0) {
    std::cerr << "Failed to initialize Winsock" << std::endl;
    shutdown();
    return false;
  }
#endif

  return true;
}

bool LocalDiscover::_receive(char* buffer, size_t buffer_size, sockaddr_in& client_addr) {
  socklen_t client_len = sizeof(client_addr);
  ssize_t received_bytes = recvfrom(_socket_fd, buffer, buffer_size, 0, (struct sockaddr*)&client_addr, &client_len);
  if (received_bytes == -1) {
    std::cerr << "Failed to receive data" << std::endl;
    return false;
  }
  return true;
}

bool LocalDiscover::_send(const char* buffer, size_t len, const sockaddr_in& client_addr) {
  ssize_t sent_bytes = sendto(_socket_fd, buffer, len, 0, (const struct sockaddr*)&client_addr, sizeof(client_addr));
  if (sent_bytes == -1) {
    std::cerr << "Failed to send data" << std::endl;
    return false;
  }
  return true;
}

void LocalDiscover::shutdown() {
  if (_socket_fd != -1) {
#ifdef _WIN32
    closesocket(_socket_fd);
    WSACleanup();
#else
    close(_socket_fd);
#endif
    _socket_fd = -1;
  }
}

}  // namespace Kapua
