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

UDPNetwork::UDPNetwork(Logger* logger, Config* config, Core* core, RSA* rsa) {
  _logger = new ScopedLogger("UDPNetwork", logger);
  _core = core;
  _config = config;
  _rsa = rsa;
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

  // Set broadcast enabled on sending socket
  int broadcast_enable = 1;
  if (setsockopt(_server_socket_fd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) == -1) {
    _logger->error("Failed setting send socket options (SO_BROADCAST)");
    _shutdown();
    return false;
  }

  return true;
}

void UDPNetwork::_main_loop() {
  sockaddr_in from_addr;
  Node* node;

  // Standby Buffer
  std::shared_ptr<Packet> pkt = nullptr;

  // Set up listening on the server port
  if (!_listen(_port)) {
    _logger->error("Listen failed");
    _shutdown();
    return;
  }

  // Set state _running true
  _running = true;
  _logger->debug("Started");

  // Set this in the past so we immediately do a broadcast
  auto last_broadcast_time = std::chrono::steady_clock::now() - std::chrono::hours(24);

  while (_running) {
    // If there's no standby buffer, allocate one
    if (!pkt) pkt = std::make_shared<Packet>();

    // Receive if any
    if (_receive(&node, pkt, from_addr)) {
      _process_packet(node, pkt);
    }

    // Get timing
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_broadcast_time);

    if (_config->local_discovery_enable && duration.count() >= _config->local_discovery_interval_ms) {
      // Do the discovery broadcast
      _broadcast();

      // Reset the last_broadcast_time variable
      last_broadcast_time = now;
    }
  }

  _logger->debug("Stopping...");
  _shutdown();

  _logger->debug("Stopped");
}  // namespace Kapua

void UDPNetwork::_process_packet(Node* node, std::shared_ptr<Packet> pkt) {
  uint8_t buffer[KAPUA_MAX_DATA_SIZE];
  size_t len;
  std::shared_ptr<Packet> reply;

  if (node) node->update_last_contact();

  _logger->debug("Packet From " + Util::to_hex64_str(pkt->from_id) + ", " + std::to_string(pkt->length) + " bytes (" +
                 Packet::packet_type_to_string(pkt->type) + ")");

  switch (pkt->type) {
    case Packet::Ping:
      break;

    case Packet::PublicKeyRequest:
      // The node must be known to us
      if (!node) {
        _logger->warn("PublicKeyRequest from unknown node");
        break;
      }

      // Reply with our public key
      reply = std::make_shared<Packet>(Packet::PublicKeyReply, _core->get_my_id(), node->id);
      if (!reply->write_public_key(_core->get_my_public_key())) {
        _logger->error("write_public_key failed");
        break;
      }
      _send(node, reply, node->addr);

      // Node state is now KeyExchange
      node->state = Node::State::KeyExchange;
      break;

    case Packet::PublicKeyReply:
      // The node must be known to us
      if (!node) {
        _logger->warn("PublicKeyReply from unknown node");
        break;
      }

      // The node state must be KeyExchange
      if (node->state != Node::State::KeyExchange) {
        _logger->warn("PublicKeyReply from a Node which isnt in KeyExchange");
        break;
      }

      // Read and set the public key for this node
      pkt->read_public_key(&node->keys);

      // Generate and set a random AESContext (session key) and encrypt it using the node public key, then send it to the node.
      reply = std::make_shared<Packet>(Packet::EncryptionContext, _core->get_my_id(), node->id);
      node->aes_context.generate();
      if (!_rsa->encrypt_aes_context(&(node->aes_context), node->keys.publicKey, (uint8_t*)&(reply->data), KAPUA_MAX_DATA_SIZE, &len)) {
        _logger->warn("Encrypting AESContext failed");
        break;
      }
      reply->length = len;
      _send(node, reply, node->addr);

      // Node state is now Handshake
      node->state = Node::State::Handshake;

      break;

    case Packet::EncryptionContext:
      // The node must be known to us
      if (!node) {
        _logger->warn("EncryptionContext from unknown node");
        break;
      }

      // The node state must be Handshake
      if (node->state != Node::State::Handshake) {
        _logger->warn("EncryptionContext from a Node which isnt in Handshake");
        break;
      }

      // Decrypt the AESContext sent to us using the node's public key, set it as the node session key.
      if (!_rsa->decrypt_aes_context(&(node->aes_context), _core->get_my_public_key()->privateKey, (uint8_t*)&(pkt->data), pkt->length, &len)) {
        _logger->warn("Decrypting AESContext failed");
        break;
      }

      // Reply with the Ready message (this will now be encrypted with the session key)
      reply = std::make_shared<Packet>(Packet::Ready, _core->get_my_id(), node->id);
      reply->length = 0;
      _send(node, reply, node->addr);

      // Node state is now CheckEncryption
      node->state = Node::State::CheckEncryption;

      break;

    case Packet::Ready:
      // The node must be known to us
      if (!node) {
        _logger->warn("Ready from unknown node");
        break;
      }

      // The node state must be CheckEncryption
      if (node->state != Node::State::CheckEncryption) {
        _logger->warn("Ready from a Node which isnt in CheckEncryption");
        break;
      }

      // Node state is now Connected
      node->state = Node::State::Connected;

      break;

    case Packet::Discovery:
      // _logger->debug("Discovery from " + Util::to_hex64_str(pkt->from_id));
      break;

    default:
      _logger->warn("Unknown packet type " + std::to_string(pkt->type));
      break;
  }
}

void UDPNetwork::_broadcast() {
  std::shared_ptr<Packet> pkt = std::make_shared<Packet>(Packet::Discovery, _core->get_my_id(), KAPUA_ID_BROADCAST);

  // The broadcast address
  struct sockaddr_in broadcast_addr;
  std::memset(&broadcast_addr, 0, sizeof(broadcast_addr));
  broadcast_addr.sin_family = AF_INET;
  broadcast_addr.sin_port = htons(_port);
  broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

  // Do the send
  // _logger->debug("Discovery broadcast...");
  if (!_send(nullptr, pkt, broadcast_addr)) {
    _logger->error("Discovery broadcast error");
  }
}

bool UDPNetwork::_receive(Node** node, std::shared_ptr<Packet> pkt, sockaddr_in& client_addr) {
  uint8_t crypt_buffer[KAPUA_MAX_PACKET_SIZE];
  uint8_t* buffer = reinterpret_cast<uint8_t*>(pkt.get());

  // Do a select to see if there is a packet waiting
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 100;
  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(_server_socket_fd, &rfds);
  int recVal = select(_server_socket_fd + 1, &rfds, NULL, NULL, &tv);

  // If there is, get the packet and return
  if (recVal <= 0) {
    return false;
  }

  // Get a packet, reaad directly into packet buffer
  socklen_t client_len = sizeof(client_addr);
  uint16_t size = recvfrom(_server_socket_fd, buffer, KAPUA_MAX_PACKET_SIZE, 0, (struct sockaddr*)&client_addr, &client_len);

  // Is there a packet?
  if (size <= 0) {
    return false;
  }
  // _logger->debug("Parsing Packet (" + std::to_string(size) + " bytes)");

  // Is the packet large enough?
  if (size < KAPUA_HEADER_SIZE) {
    _logger->debug("Non-Kapua packet received (too short)");
    return false;
  }

  // Packet may be from a known node.
  *node = _core->find_node(client_addr);

  // Valid magic Number?
  if (pkt->check_magic_valid()) {
    // If yes, the packet is unencrypted.
    if (*node && (*node)->state >= Node::State::Connected && pkt->type != Packet::PacketType::Discovery) {
      _logger->debug("Warning: Connected node sent us an unencrypted packet");
    }
  } else {
    // Packet is either encrypted, or bad. Try a decrypt
    if (*node && (*node)->state >= Node::State::CheckEncryption) {
      // Check for connected/context
      _logger->warn("Decrypting packet");

      if (!_aes_decrypt((*node)->aes_context, buffer, size, crypt_buffer)) {
        _logger->error("Error while decrypting packet");
        return false;
      }

      // Update packet with unencrypted plaintext
      memcpy(buffer, crypt_buffer, size);

      // Check now valid
      if (!pkt->check_magic_valid()) {
        _logger->debug("Error: Decrypted packet has bad magic number");
        // TODO: Handle node state.
        // node->state = Node::State::Desynchronisied;
        return false;
      } else {
        _logger->debug("Decrypted packet OK");
      }
    }
  }

  // Check Version
  if (!pkt->check_version_valid()) {
    // TODO: Setting for strict version checking
    // Version
    _logger->debug("Packet received with incompatible version (" + pkt->get_version_string() + ")");
    return false;
  }

  // Check from us
  if (pkt->from_id == _core->get_my_id()) {
    // _logger->debug("Packet received from own ID");
    // Ignore packets from us
    return false;
  }

  std::string client_addr_str = Util::sockaddr_to_string(client_addr);

  // Is this a new node?
  if (!*node) {
    // Add the node
    *node = _core->add_node(pkt->from_id, client_addr);
    _logger->info("New node detected, ID: " + Util::to_hex64_str(pkt->from_id) + " (" + client_addr_str + ")");

    std::shared_ptr<Packet> rpk_pkt = std::make_shared<Packet>(Packet::PublicKeyRequest, _core->get_my_id(), pkt->from_id);

    _logger->debug("Sending PublicKeyRequest to NodeID " + std::to_string((*node)->id) + " (" + client_addr_str + ")");
    if (!_send((*node), rpk_pkt, client_addr)) {
      _logger->warn("Error Sending PublicKeyRequest to NodeID " + std::to_string((*node)->id) + " (" + client_addr_str + ")");
    }
  }

  return true;
}

bool UDPNetwork::_send(Node* node, std::shared_ptr<Packet> pkt, const sockaddr_in& addr) {
  uint8_t crypt_buffer[KAPUA_MAX_PACKET_SIZE];
  uint8_t* buffer = reinterpret_cast<uint8_t*>(pkt.get());
  if (node != nullptr && node->state >= Node::State::CheckEncryption) {
    _logger->warn("Encrypting packet");
    if (!_aes_encrypt(node->aes_context, buffer, KAPUA_HEADER_SIZE + pkt->length, crypt_buffer)) {
      _logger->error("Error while encrypting packet");
      return false;
    }
    buffer = crypt_buffer;
  }

  return sendto(_server_socket_fd, buffer, KAPUA_HEADER_SIZE + pkt->length, 0, (const struct sockaddr*)&addr, sizeof(addr)) == KAPUA_HEADER_SIZE + pkt->length;
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

#ifdef _WIN32
  // Cleanup if Windows
  WSACleanup();
#endif

  return true;
}

// AES encryption and decryption functions
bool UDPNetwork::_aes_encrypt(AESContext& context, const uint8_t* plaintext, size_t plaintext_len, uint8_t* ciphertext) {
  EVP_CIPHER_CTX* ctx;
  int len;
  int ciphertext_len;

  // Create and initialize the context
  if (!(ctx = EVP_CIPHER_CTX_new())) return false;

  // Initialize the encryption operation
  if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, context.key, context.iv) != 1) {
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

bool UDPNetwork::_aes_decrypt(AESContext& context, const uint8_t* ciphertext, size_t ciphertext_len, uint8_t* plaintext) {
  EVP_CIPHER_CTX* ctx;
  int len;
  int plaintext_len;

  // Create and initialize the context
  if (!(ctx = EVP_CIPHER_CTX_new())) return false;

  // Initialize the decryption operation
  if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, context.key, context.iv) != 1) {
    _logger->warn("EVP_DecryptInit_ex failed");
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }

  // Provide the ciphertext to be decrypted
  if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len) != 1) {
    _logger->warn("EVP_DecryptUpdate failed");
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }
  plaintext_len = len;

  // Finalize the decryption
  if (EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1) {
    _logger->warn("EVP_DecryptFinal_ex failed");
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }
  plaintext_len += len;

  // Clean up
  EVP_CIPHER_CTX_free(ctx);

  return true;
}

}  // namespace Kapua
