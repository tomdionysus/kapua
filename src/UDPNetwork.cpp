//
// Kapua UDPNetwork class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#include "UDPNetwork.hpp"

#include "Protocol.hpp"
#include "Util.hpp"

namespace Kapua {

UDPNetwork::UDPNetwork(Logger* logger, Core* core) {
  _logger = new ScopedLogger("UDPNetwork", logger);
  _core = core;
  _running = false;
}

UDPNetwork::~UDPNetwork() {
  if (_running) stop();
  delete _logger;
}

bool UDPNetwork::start(int port) {
  _logger->debug("Starting...");
  if (_running) {
    _logger->warn("start called, but thread already running");
    return false;
  }

  _port = port;
  _main_thread = new std::thread(&UDPNetwork::_main_loop, this);
  return true;
}

bool UDPNetwork::stop() {
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

bool UDPNetwork::_listen(int port) {
#ifdef _WIN32
  // Initialize Windows Socket API (Winsock)
  if (WSAStartup(MAKEWORD(2, 2), &_wsaData) != 0) {
    _logger->error("WIN32: Failed to initialize Winsock");
    _shutdown();
    return false;
  }
#endif

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

  // Set broadcast enabled on sending socket
  int broadcast_enable = 1;
  if (setsockopt(_client_socket_fd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) == -1) {
    _logger->error("Failed setting send socket options (SO_BROADCAST)");
    _shutdown();
    return false;
  }

  return true;
}

void UDPNetwork::_main_loop() {
  sockaddr_in from_addr;

  // Standby Buffer
  std::shared_ptr<Packet> pkt = nullptr;

  // Set state _running true
  _running = true;

  // Set up listening on the server port
  if (!_listen(_port)) {
    _logger->error("Listen failed");
    _shutdown();
    return;
  }

  // Set this in the past so we immediately do a broadcast
  auto last_broadcast_time = std::chrono::steady_clock::now() - std::chrono::hours(24);

  while (_running) {
    // If there's no standby buffer, allocate one
    if (!pkt) pkt = std::make_shared<Packet>();

    // Receive if any
    ssize_t len = _receive(reinterpret_cast<char*>(pkt.get()), 65535, from_addr);
    if (len < 0) {
      _logger->error("Server receive error: " + std::string(strerror(errno)));
    } else if (len > 0) {
      std::string from_addr_str = std::string(inet_ntoa(from_addr.sin_addr)) + ":" + std::to_string(ntohs(from_addr.sin_port));
      //_logger->debug("Packet From " + from_addr_str + ", " + std::to_string(len) + " bytes.");

      // Verify the packet
      if (len < 32) {
        // Length First
        _logger->debug("Non-Kapua packet received (too short)");
      } else {
        // Valid magic Number?
        if (!pkt->check_magic_valid()) {
          std::string bad_magic = [&] {
            std::ostringstream oss;
            oss << "0x" << std::setfill('0') << std::setw(10) << std::hex << pkt->magic;
            return oss.str();
          }();
          _logger->debug("Non-Kapua packet received (bad magic number " + bad_magic + ")");
        } else if (!pkt->check_version_valid()) {
          // TODO: Setting for strict version checking
          // Version
          _logger->debug("Packet received with incompatible version (" + pkt->get_version_string() + ")");
        } else if (pkt->from_id == _core->get_my_id()) {
          // Ignore packets from us
        } else {
          // Check ID does not exist
          Node* node = _core->find_node(pkt->from_id);
          if (!node) {
            node = _core->add_node(pkt->from_id, from_addr);
            _logger->info("New node detected, ID: " + Util::to_hex64_str(pkt->from_id) + " (" + from_addr_str + ")");
          } 
          _process_packet(node, pkt);
        }
      }
    }

    // Get timing
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - last_broadcast_time);

    if (duration.count() >= 5) {
      // Do the discovery broadcast
      _broadcast();

      // Reset the last_broadcast_time variable
      last_broadcast_time = now;
    }
  }

  _logger->debug("Stopping...");
  _shutdown();

  _logger->debug("Stopped");
}

void UDPNetwork::_process_packet(Node* node, std::shared_ptr<Packet> pkt) {
  // _logger->error("Processing Packet");

  node->update_last_contact();
}

void UDPNetwork::_broadcast() {
  std::unique_ptr<Packet> pkt = std::make_unique<Packet>();

  pkt->from_id = _core->get_my_id();
  pkt->to_id = KAPUA_ID_BROADCAST;
  pkt->length = 0;

  // The broadcast address
  struct sockaddr_in broadcast_addr;
  std::memset(&broadcast_addr, 0, sizeof(broadcast_addr));
  broadcast_addr.sin_family = AF_INET;
  broadcast_addr.sin_port = htons(_port);
  broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

  // Do the send
  // _logger->debug("Discovery broadcast...");
  ssize_t res = _send(reinterpret_cast<char*>(pkt.get()), sizeof(Packet), broadcast_addr);

  if (res == -1) {
    _logger->error("Discovery broadcast error: " + std::string(strerror(errno)));
  }
}

ssize_t UDPNetwork::_receive(char* buffer, size_t buffer_size, sockaddr_in& client_addr) {
  // Do a select to see if there is a packet waiting
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 100;
  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(_server_socket_fd, &rfds);
  int recVal = select(_server_socket_fd + 1, &rfds, NULL, NULL, &tv);

  // If there is, get the packet and return
  if (recVal > 0) {
    socklen_t client_len = sizeof(client_addr);
    return recvfrom(_server_socket_fd, buffer, buffer_size, 0, (struct sockaddr*)&client_addr, &client_len);
  } else {
    return 0;
  }
}

ssize_t UDPNetwork::_send(const char* buffer, size_t len, const sockaddr_in& client_addr) {
  return sendto(_client_socket_fd, buffer, len, 0, (const struct sockaddr*)&client_addr, sizeof(client_addr));
}

bool UDPNetwork::_shutdown() {
  // Close the server socket
  if (_server_socket_fd != -1) {
#ifdef _WIN32
    closesocket(_server_socket_fd);
#else
    close(_server_socket_fd);
#endif
    _server_socket_fd = -1;
  }

  // Close the client socket
  if (_client_socket_fd != -1) {
#ifdef _WIN32
    closesocket(_client_socket_fd);
#else
    close(_client_socket_fd);
#endif
    _client_socket_fd = -1;
  }

#ifdef _WIN32
  // Cleanup if Windows
  WSACleanup();
#endif

  return true;
}

// AES CBC functions

// AES encryption and decryption functions
bool UDPNetwork::_aes_encrypt(const unsigned char* key, const unsigned char* iv, const uint8_t* plaintext, size_t plaintext_len, uint8_t* ciphertext) {
  EVP_CIPHER_CTX* ctx;
  int len;
  int ciphertext_len;

  // Create and initialize the context
  if (!(ctx = EVP_CIPHER_CTX_new())) return false;

  // Initialize the encryption operation
  if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv) != 1) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }

  // Provide the plaintext to be encrypted
  if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len) != 1) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }
  ciphertext_len = len;

  // Finalize the encryption
  if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }
  ciphertext_len += len;

  // Clean up
  EVP_CIPHER_CTX_free(ctx);

  return true;
}

bool UDPNetwork::_aes_decrypt(const unsigned char* key, const unsigned char* iv, const uint8_t* ciphertext, size_t ciphertext_len, uint8_t* plaintext) {
  EVP_CIPHER_CTX* ctx;
  int len;
  int plaintext_len;

  // Create and initialize the context
  if (!(ctx = EVP_CIPHER_CTX_new())) return false;

  // Initialize the decryption operation
  if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv) != 1) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }

  // Provide the ciphertext to be decrypted
  if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len) != 1) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }
  plaintext_len = len;

  // Finalize the decryption
  if (EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }
  plaintext_len += len;

  // Clean up
  EVP_CIPHER_CTX_free(ctx);

  return true;
}

// Examples.

// int main() {
//     // AES-256 key (32 bytes)
//     unsigned char aes_key[32];
//     // Securely generate a random AES key
//     if (RAND_bytes(aes_key, sizeof(aes_key)) != 1) {
//         std::cerr << "Error generating AES key." << std::endl;
//         return 1;
//     }

//     // Generate a secure random IV (Initialization Vector) for AES
//     unsigned char iv[EVP_MAX_IV_LENGTH];
//     if (RAND_bytes(iv, EVP_MAX_IV_LENGTH) != 1) {
//         std::cerr << "Error generating IV." << std::endl;
//         return 1;
//     }

//     // Message to encrypt
//     const char* plaintext = "Hello, OpenSSL AES-256 CBC Encryption!";

//     // Ensure the plaintext is a multiple of the block size (16 bytes for AES)
//     size_t plaintext_len = strlen(plaintext);
//     size_t padded_len = (plaintext_len + 15) & ~15;  // Round up to the nearest multiple of 16
//     uint8_t padded_plaintext[padded_len];
//     memset(padded_plaintext, 0, sizeof(padded_plaintext));
//     memcpy(padded_plaintext, plaintext, plaintext_len);

//     // Buffer for ciphertext and decrypted text
//     uint8_t ciphertext[padded_len];
//     uint8_t decrypted[padded_len];

//     // Encrypt the plaintext
//     if (!aes_encrypt(aes_key, iv, padded_plaintext, padded_len, ciphertext)) {
//         std::cerr << "Error encrypting." << std::endl;
//         return 1;
//     }

//     // Decrypt the ciphertext
//     if (!aes_decrypt(aes_key, iv, ciphertext, padded_len, decrypted)) {
//         std::cerr << "Error decrypting." << std::endl;
//         return 1;
//     }

//     // Display the results
//     std::cout << "Original Text: " << plaintext << std::endl;
//     std::cout << "Encrypted Text: ";
//     for (size_t i = 0; i < padded_len; ++i) {
//         std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)ciphertext[i];
//     }
//     std::cout << std::endl
// }

}  // namespace Kapua
