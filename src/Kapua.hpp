//
// Kapua Definitons
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#pragma once

namespace Kapua {

const std::array<uint8_t, 5> KAPUA_MAGIC_NUMBER = {0x4B, 0x61, 0x70, 0x75, 0x61};

struct KapuaVersion {
  uint8_t major;
  uint8_t minor;
  uint8_t patch;
};

const KapuaVersion KAPUA_VERSION = {0x00, 0x00, 0x01};

}  // namespace Kapua