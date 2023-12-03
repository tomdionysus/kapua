#pragma once

#include <cstring>
#include <iostream>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "Logger.hpp"

namespace Kapua {
class LocalDiscover {
 public:
  LocalDiscover(Logger* logger);
  ~LocalDiscover();

  bool start(int port);
  bool receive(char* buffer, size_t buffer_size, sockaddr_in& client_addr);
  bool send(const char* message, const sockaddr_in& client_addr);
  void shutdown();

 private:
  int _socket_fd;
  sockaddr_in _server_addr;

  Logger* _logger;

#ifdef _WIN32
  WSADATA _wsaData;
#endif
};
}  // namespace Kapua
