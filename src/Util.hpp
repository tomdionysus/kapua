//
// Kapua Util functions
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
namespace Kapua {

namespace Util {

static std::string to_hex64_str(uint64_t val) {
  std::ostringstream oss;
  oss << "0x" << std::setfill('0') << std::setw(16) << std::hex << val;
  return oss.str();
}

}  // namespace Util

}  // namespace Kapua