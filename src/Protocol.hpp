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

#define KAPUA_HEADER_SIZE 44

  // --- This is the end of header

  uint8_t data[KAPUA_MAX_PACKET_SIZE - KAPUA_HEADER_SIZE];

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

  Packet(uint8_t* buffer, uint16_t length) { memcpy(this, buffer, length); }

  bool check_magic_valid() { return std::memcmp(magic, KAPUA_MAGIC_NUMBER.data(), KAPUA_MAGIC_NUMBER.size()) == 0; }

  bool check_version_valid(bool strict = false) {
    if (version.major != KAPUA_VERSION.major) return false;
    if (strict && version.minor > KAPUA_VERSION.minor) return false;
    return true;
  }

  bool is_reply() { return request_id != KAPUA_ID_NULL; }

  std::string get_version_string() { return std::to_string(version.major) + "." + std::to_string(version.minor) + "." + std::to_string(version.patch); }

  // Packet PublicKeyReply
  void write_public_key(KeyPair& key_pair) {
    length = sizeof(KeyPair::publicKey);
    std::memcpy(&data, &key_pair.publicKey, length);
  }

  bool read_public_key(KeyPair& key_pair) {
    if (length != sizeof(KeyPair::publicKey)) return false;
    std::memcpy(&key_pair.publicKey, &data, length);
    return true;
  }
};
#pragma pack(pop)

}  // namespace Kapua