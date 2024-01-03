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

namespace Kapua {

#define KAPUA_MAX_PACKET_SIZE 1450
#define KAPUA_ID_GROUP 0xFFFFFFFFFFFFFF01
#define KAPUA_ID_BROADCAST 0xFFFFFFFFFFFFFFFF

typedef struct Peer {
  uint64_t id;
  sockaddr_in addr;
} Peer_t;

#pragma pack(push, 1)
struct Packet {
  enum PacketType : uint16_t {
    Ping,
    Pong,
    PublicKey,
    EncryptionContext,
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

  Packet() {
    std::memcpy(magic, KAPUA_MAGIC_NUMBER.data(), KAPUA_MAGIC_NUMBER.size());
    version = KAPUA_VERSION;
  }

  bool check_magic_valid() { return std::memcmp(magic, KAPUA_MAGIC_NUMBER.data(), KAPUA_MAGIC_NUMBER.size()) == 0; }
  bool check_version_valid(bool strict = false) {
    if (version.major != KAPUA_VERSION.major) return false;
    if (strict && version.minor > KAPUA_VERSION.minor) return false;
    return true;
  }
  std::string get_version_string() { return std::to_string(version.major) + "." + std::to_string(version.minor) + "." + std::to_string(version.patch); }
};

#pragma pack(pop)

}  // namespace Kapua