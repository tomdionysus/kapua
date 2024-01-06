//
// Kapua Util functions
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
namespace Kapua {

namespace Util {

static std::string to_hex64_str(const uint64_t &val) {
  std::ostringstream oss;
  oss << "0x" << std::setfill('0') << std::setw(16) << std::hex << val;
  return oss.str();
}

static std::string sockaddr_to_string(const sockaddr_in &addr) { return std::string(inet_ntoa(addr.sin_addr)) + ":" + std::to_string(ntohs(addr.sin_port)); }

static std::string to_hex(const uint8_t *buffer, size_t len) {
  std::ostringstream oss;
  oss << std::setfill('0') << std::setw(2) << std::hex;
  for(uint32_t i= 0; i< len; i++) {
    oss << std::setw(2) << static_cast<unsigned int>(buffer[i]);
  }
 return oss.str();
}

}  // namespace Util

}  // namespace Kapua