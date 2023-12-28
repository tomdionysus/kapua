//
// Kapua Settings class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#include "Settings.hpp"

#include "Logger.hpp"

using namespace std;

namespace Kapua {

Settings::Settings(Logger* logger) { _logger = new ScopedLogger("Settingsuration", logger); }

Settings::~Settings() { delete _logger; }

bool Settings::load(std::string filename = "Settings.yaml") {
  _logger->debug("Loading Settings file " + _filename);
  return true;
}

bool Settings::save(std::string filename) {
  _logger->debug("Saving Settings file " + _filename);
  return true;
}

}  // namespace Kapua