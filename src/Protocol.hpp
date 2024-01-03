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
    Encryption,
  };

  uint8_t magic[5];
  KapuaVersion version;

  PacketType type;
  uint64_t packet_id;
  uint64_t from_id;
  uint64_t to_id;
  uint16_t ttl = 32;
  uint16_t length;

  uint8_t data[KAPUA_MAX_PACKET_SIZE-38];

  Packet() {
    std::memcpy(magic, KAPUA_MAGIC_NUMBER.data(), KAPUA_MAGIC_NUMBER.size());
    version = KAPUA_VERSION;
  }

  bool isMagicValid() { return std::memcmp(magic, KAPUA_MAGIC_NUMBER.data(), KAPUA_MAGIC_NUMBER.size()) == 0; }
  bool isVersionValid(bool strict = false) {
    if (version.major != KAPUA_VERSION.major) return false;
    if (strict && version.minor > KAPUA_VERSION.minor) return false;
    return true;
  }
  std::string getVersionString() { return std::to_string(version.major) + "." + std::to_string(version.minor) + "." + std::to_string(version.patch); }
};

#pragma pack(pop)

}  // namespace Kapua