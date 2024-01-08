//
// Kapua UDPNetwork class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#pragma once

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
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

#include "Config.hpp"
#include "Core.hpp"
#include "Kapua.hpp"
#include "Logger.hpp"
#include "Protocol.hpp"
#include "RSA.hpp"

namespace Kapua {
class UDPNetwork {
 public:
  UDPNetwork(Logger* logger, Config* config, Core* core, RSA* rsa);
  ~UDPNetwork();

  bool start(int port);
  bool stop();

 protected:
  bool _listen(int port);
  void _main_loop();
  void _broadcast();
  bool _send(Node* node, std::shared_ptr<Packet> pkt, const sockaddr_in& addr);
  bool _receive(Node** node, std::shared_ptr<Packet> pkt, sockaddr_in& client_addr);
  bool _shutdown();

  bool _aes_encrypt(AESKey& context, const uint8_t* plaintext, size_t plaintext_len, uint8_t* ciphertext, size_t *ciphertext_len);
  bool _aes_decrypt(AESKey& context, const uint8_t* ciphertext, size_t ciphertext_len, uint8_t* plaintext, size_t *plaintext_len);
  std::string get_aes_error_string();

  void _generate_iv(uint8_t* ptr) {
    RAND_bytes(ptr, 32);
  }

  void _process_packet(Node* node, std::shared_ptr<Packet> packet);

  Core* _core;
  Config* _config;
  RSA* _rsa;

  uint16_t _port;

  int _server_socket_fd;
  sockaddr_in _server_addr;

  Logger* _logger;

  std::thread* _main_thread;

  std::atomic_bool _running;

#ifdef _WIN32
  WSADATA _wsaData;
#endif
};
}  // namespace Kapua
