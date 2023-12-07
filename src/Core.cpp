//
// Kapua Core class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023 
//
#include "Core.hpp"

#include "Logger.hpp"

using namespace std;

namespace Kapua {
Core ::Core(Logger* logger, Config* config) {
  _logger = logger;
  _config = config;
  _logger->debug("Starting...");
  _logger->debug("Loading config...");
  _config->load();

  _logger->debug("Started");
}

Core ::~Core() { _logger->debug("Stopped"); }

void Core::add_node(uint64_t id, Node* node) {
  std::lock_guard<std::mutex> lock(_nodes_mutex);
  _nodes.insert({id, node});
}

void Core::remove_node(uint64_t id) {
  std::lock_guard<std::mutex> lock(_nodes_mutex);
  _nodes.erase(id);
}

Node* Core::find_node(uint64_t id) {
  std::lock_guard<std::mutex> lock(_nodes_mutex);
  auto search = _nodes.find(id);
  if (search != _nodes.end()) return search->second;
  return nullptr;
}

void get_version(Version_t* version) {
  version->major = KAPUA_VERSION_MAJOR;
  version->minor = KAPUA_VERSION_MINOR;
  version->patch = KAPUA_VERSION_PATCH;
}

}  // namespace Kapua
