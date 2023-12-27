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

class Config {
 public:
  Config(Logger* logger, std::string filename);
  ~Config();

  bool load();
  bool dump();

 protected:
  Logger* _logger;
  std::string _filename;
};

};  // namespace Kapua
