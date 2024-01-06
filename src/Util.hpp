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

}  // namespace Util

}  // namespace Kapua