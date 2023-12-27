//
// Kapua Config class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#include "Config.hpp"

#include "Logger.hpp"

using namespace std;

namespace Kapua {

Config::Config(Logger* logger, std::string filename) {
  _logger = new ScopedLogger("Configuration", logger);
  _filename = filename;
}

Config::~Config() {
  delete _logger;
}

bool Config::load() {
  _logger->debug("Loading config file " + _filename);
  return true;
}

bool Config::dump() {
  _logger->debug("Printing config");
  return true;
}

}  // namespace Kapua