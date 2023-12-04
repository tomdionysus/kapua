#pragma once

#include <cstring>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

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
  bool stop();

 private:
  bool _listen(int port);
  void _main_loop();
  ssize_t _receive(char* buffer, size_t buffer_size, sockaddr_in& client_addr);
  ssize_t _send(const char* buffer, size_t len, const sockaddr_in& client_addr);
  bool _shutdown();

  int _socket_fd;
  sockaddr_in _server_addr;

  Logger* _logger;

  std::thread* _main_thread;
  std::mutex _running_mutex;
  bool _running;

#ifdef _WIN32
  WSADATA _wsaData;
#endif
};
}  // namespace Kapua
