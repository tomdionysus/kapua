//
// Kapua Protocol classes
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#pragma once

#include <cstdint>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace Kapua {

#define KAPUA_MAGIC_NUMBER 0x6b617075
#define KAPUA_ID_GROUP 0xFFFFFFFFFFFFFF01
#define KAPUA_ID_BROADCAST 0xFFFFFFFFFFFFFFFF
#define KAPUA_PORT 11860

typedef struct Peer {
  uint64_t id;
  sockaddr_in addr;
} Peer_t;

struct Packet {
  enum PacketType : uint16_t {
    Ping,
    Pong,
  };

  std::uint32_t magic = KAPUA_MAGIC_NUMBER;
  PacketType type;
  std::uint64_t packet_id;
  std::uint64_t from_id;
  std::uint64_t to_id;
  std::uint16_t ttl = 32;
  std::uint16_t length;

  union {
    struct {
      Peer_t peer[];
    } peer_list;
  };
};

}  // namespace Kapua