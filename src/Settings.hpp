//
// Kapua Config class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#pragma once

#include <yaml.h>

#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

#include "Logger.hpp"

namespace Kapua {

class Settings {
  Settings(Logger* _logger, Core* _core);
  bool writeSettings(uint64_t id, const std::vector<Item>& items);
  bool readSettings(uint64_t& id, std::vector<Item>& items);

 private:
  void writeSettingsSection(yaml_emitter_t* emitter, uint64_t id, const std::vector<Item>& items);
  void writeUint64Field(yaml_emitter_t* emitter, const char* fieldName, uint64_t value);
  void writeStringField(yaml_emitter_t* emitter, const char* fieldName, const std::string& value);

  std::string sockaddr_inToString(sockaddr_in address);
  bool parseSettings(yaml_parser_t* parser, uint64_t& id, std::vector<Item>& items);
  bool parseUint64Field(yaml_parser_t* parser, uint64_t& value);
  bool parseItems(yaml_parser_t* parser, std::vector<Item>& items);
  bool parseItem(yaml_parser_t* parser, Item& item);
  bool parseAddress(yaml_parser_t* parser, sockaddr_in& address);
};

}  // namespace Kapua
