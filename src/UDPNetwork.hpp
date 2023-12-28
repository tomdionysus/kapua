//
// Kapua UDPNetwork class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#pragma once

#include <cstring>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
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

#include "Core.hpp"
#include "Kapua.hpp"
#include "Logger.hpp"

namespace Kapua {
class UDPNetwork {
 public:
  UDPNetwork(Logger* logger, Core* core);
  ~UDPNetwork();

  bool start(int port);
  bool stop();

 private:
  bool _listen(int port);
  void _main_loop();
  void _broadcast();
  ssize_t _receive(char* buffer, size_t buffer_size, sockaddr_in& client_addr);
  ssize_t _send(const char* buffer, size_t len, const sockaddr_in& client_addr);
  bool _shutdown();

  Core* _core;

  uint16_t _port;

  int _server_socket_fd;
  int _client_socket_fd;
  sockaddr_in _server_addr;

  Logger* _logger;

  std::thread* _main_thread;

  std::atomic_bool _running;

#ifdef _WIN32
  WSADATA _wsaData;
#endif
};
}  // namespace Kapua
