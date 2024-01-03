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
  _logger = new Kapua::ScopedLogger("Core", logger);

  _config = config;
}

Core ::~Core() {
  // Free all Nodes
  for (const auto& pair : _nodes) {
    uint64_t key = pair.first;
    delete pair.second;
  }

  _logger->debug("Stopped");
}

bool Core::start() {
  _logger->debug("Starting...");
  _my_id = _get_random_id();
  _logger->debug("Started");
  return true;
}

void Core::add_node(uint64_t id, sockaddr_in addr) {
  std::lock_guard<std::mutex> lock(_nodes_mutex);
  Node* node = new Node(id, addr);
  _nodes.insert({id, node});
  _nodes_by_addr.insert({(SockaddrHashable)addr, node});
}

void Core::remove_node(uint64_t id) {
  std::lock_guard<std::mutex> lock(_nodes_mutex);
  Node* node = find_node(id);
  if (!node) return;
  _nodes_by_addr.erase(node->addr);
  _nodes.erase(id);
  delete node;
}

Node* Core::find_node(uint64_t id) {
  std::lock_guard<std::mutex> lock(_nodes_mutex);
  auto search = _nodes.find(id);
  if (search != _nodes.end()) return search->second;
  return nullptr;
}

Node* Core::find_node(sockaddr_in addr) {
  std::lock_guard<std::mutex> lock(_nodes_mutex);
  auto search = _nodes_by_addr.find(addr);
  if (search != _nodes_by_addr.end()) return search->second;
  return nullptr;
}

uint64_t Core::get_my_id() { return _my_id; }

uint64_t Core::_get_random_id() {
  std::random_device rd;
  std::default_random_engine generator(rd());
  std::uniform_int_distribution<uint64_t> distribution(std::numeric_limits<uint64_t>::min(), std::numeric_limits<uint64_t>::max());
  return distribution(generator);
}

void get_version(Version_t* version) {
  version->major = KAPUA_VERSION_MAJOR;
  version->minor = KAPUA_VERSION_MINOR;
  version->patch = KAPUA_VERSION_PATCH;
}

}  // namespace Kapua
