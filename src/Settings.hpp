//
// Kapua Config class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#pragma once

#include <cstdint>
#include <vector>
#include <map>

#include "Logger.hpp"

namespace Kapua {

class Settings {
 public:
  Settings(Logger* logger);
  ~Settings();

  bool load(std::string filename);
  bool save(std::string filename);

 protected:
  Logger* _logger;
  std::string _filename;
};

};  // namespace Kapua
