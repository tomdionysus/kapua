//
// Kapua Protocol classes
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "Kapua.hpp"
#include "RSA.hpp"

namespace Kapua {

#define KAPUA_MAX_PACKET_SIZE 1450
#define KAPUA_HEADER_SIZE 46
#define KAPUA_MAX_DATA_SIZE (KAPUA_MAX_PACKET_SIZE - KAPUA_HEADER_SIZE)

#define KAPUA_ID_GROUP 0xFFFFFFFFFFFFFF01
#define KAPUA_ID_BROADCAST 0xFFFFFFFFFFFFFFFF
#define KAPUA_ID_NULL 0x0000000000000000

typedef struct Peer {
  uint64_t id;
  sockaddr_in addr;
} Peer_t;

#pragma pack(push, 1)
struct Packet {
  enum PacketType : uint16_t {
    Ping,
    PublicKeyRequest,
    PublicKeyReply,
    EncryptionContext,
    Ready,

    Discovery = 0xFFFF,
  };

  // --- This is the start of the header

  uint8_t magic[5];
  KapuaVersion version;

  PacketType type;
  uint64_t packet_id;   // This packet ID
  uint64_t from_id;     // The originating Node ID
  uint64_t to_id;       // The destination Node ID
  uint16_t ttl = 32;    // The time to live (should be decremented on forward)
  uint64_t request_id;  // The request packet ID, if this is a reply, or 0x0000000000000000
  uint16_t length = 0;  // The length of the data to follow


  // --- This is the end of header

  uint8_t data[KAPUA_MAX_DATA_SIZE];

  // Methods

  Packet() {
    std::memcpy(magic, KAPUA_MAGIC_NUMBER.data(), KAPUA_MAGIC_NUMBER.size());
    version = KAPUA_VERSION;
    length = 0;
  }

  Packet(PacketType pType, uint64_t fromId) : Packet() {
    type = pType;
    from_id = fromId;
    to_id = KAPUA_ID_BROADCAST;
  }

  Packet(PacketType pType, uint64_t fromId, uint64_t toId) : Packet(pType, fromId) { to_id = toId; }

  Packet(PacketType pType, uint64_t fromId, uint64_t toId, uint64_t requestId) : Packet(pType, fromId, toId) { request_id = requestId; }

  bool check_magic_valid() { return std::memcmp(magic, KAPUA_MAGIC_NUMBER.data(), KAPUA_MAGIC_NUMBER.size()) == 0; }

  bool check_version_valid(bool strict = false) {
    if (version.major != KAPUA_VERSION.major) return false;
    if (strict && version.minor > KAPUA_VERSION.minor) return false;
    return true;
  }

  bool is_reply() { return request_id != KAPUA_ID_NULL; }

  std::string get_version_string() { return std::to_string(version.major) + "." + std::to_string(version.minor) + "." + std::to_string(version.patch); }

  // Packet PublicKeyReply
  bool write_public_key(KeyPair *key_pair) {
    if (key_pair->publicKey == nullptr) {
      // Handle error: public key is not initialized
      throw std::runtime_error("Public key is null");
    }

    int len = i2d_PublicKey(key_pair->publicKey, nullptr);
    if (len <= 0) {
      // Handle error: failed to get the length
      throw std::runtime_error("Failed to get the length of the public key");
    }

    if (len > KAPUA_MAX_DATA_SIZE) {
      throw std::runtime_error("Length of encoded public key is larger than KAPUA_MAX_DATA_SIZE");
    }

    unsigned char *buffer = (unsigned char *)&data;
    unsigned char **ptrToBuffer = &buffer;

    if (i2d_PublicKey(key_pair->publicKey, ptrToBuffer) != len) {
      // Handle error: failed to serialize the key
      throw std::runtime_error("Failed to serialize the public key");
    }

    length = len;

    return true;
  }

  bool read_public_key(KeyPair *key_pair) {
    const unsigned char *buf = (unsigned char *)&data;

    key_pair->publicKey = d2i_PublicKey(EVP_PKEY_RSA, nullptr, &buf, length);
    if (key_pair->publicKey == nullptr) {
      // Handle error: deserialization failed
      throw std::runtime_error("Failed to deserialize the public key");
    }

    return true;
  }

  static const std::string packet_type_to_string(PacketType pt) {
    switch (pt) {
      case PacketType::Ping:
        return "Ping";
      case PacketType::PublicKeyRequest:
        return "PublicKeyRequest";
      case PacketType::PublicKeyReply:
        return "PublicKeyReply";
      case PacketType::EncryptionContext:
        return "EncryptionContext";
      case PacketType::Ready:
        return "Ready";
      case PacketType::Discovery:
        return "Discovery";
      default:
        return "Unknown";
    }
  }
};
#pragma pack(pop)

}  // namespace Kapua