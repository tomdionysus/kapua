//
// Kapua protocol classes and structs
//
// Author: Tom Cully <mail@tomcully.com>
//
#include <cstdint>

namespace Kapua {

#define KAPUA_MAGIC_NUMBER 0x6b617075


struct Packet {
  
  enum PacketType : uint16_t {
    Ping,
    Pong,
  };

  std::uint32_t magic = KAPUA_MAGIC_NUMBER;
  PacketType type;
  std::uint64_t from_id;
  std::uint64_t to_id;
  std::uint16_t ttl;
  std::uint16_t length;
};

}  // namespace Kapua