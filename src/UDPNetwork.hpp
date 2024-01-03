//
// Kapua UDPNetwork class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#pragma once

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <sys/time.h>

#include <atomic>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
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
#include "Protocol.hpp"
#include "RSA.hpp"

namespace Kapua {
class UDPNetwork {
 public:
  UDPNetwork(Logger* logger, Core* core);
  ~UDPNetwork();

  bool start(int port);
  bool stop();

 protected:
  bool _listen(int port);
  void _main_loop();
  void _broadcast();
  ssize_t _receive(char* buffer, size_t buffer_size, sockaddr_in& client_addr);
  bool _send(Node* node, std::shared_ptr<Packet> pkt, const sockaddr_in& addr);
  bool _shutdown();

  bool _aes_encrypt(AESContext& context, const uint8_t* plaintext, size_t plaintext_len, uint8_t* ciphertext);
  bool _aes_decrypt(AESContext& context, const uint8_t* ciphertext, size_t ciphertext_len, uint8_t* plaintext);

  void _process_packet(Node* node, std::shared_ptr<Packet> packet);

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
