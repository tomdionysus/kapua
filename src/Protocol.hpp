//
// Kapua protocol classes and structs
//
// Author: Tom Cully <mail@tomcully.com>
//
#include <cstdint>

namespace Kapua {

#define KAPUA_MAGIC_NUMBER 0x6b617075

enum PacketType : uint16_t {
  Ping,
  Pong,
};

struct Packet {
  std::uint32_t magic = KAPUA_MAGIC_NUMBER;
  PacketType type;
  std::uint64_t fromId;
  std::uint64_t toId;
  std::uint16_t ttl;
};

}  // namespace Kapua