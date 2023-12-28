//
// Kapua Config class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#pragma once

#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Logger.hpp"

namespace Kapua {

class Config {
 public:
  Config(Logger* logger, std::string filename);
  ~Config();

  bool load();
  bool dump();

  enum class ParseResult { Success, InvalidFormat, InvalidUnit };

 protected:
  Logger* _logger;
  std::string _filename;

  ParseResult parse_duration(const std::string& input, long long& milliseconds);
};

};  // namespace Kapua
