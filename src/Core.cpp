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
Core ::Core(Logger* logger, Config* config, RSA* rsa) : _running(false) {
  _logger = new Kapua::ScopedLogger("Core", logger);
  _config = config;
  _rsa = rsa;
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
  _thread = boost::thread(&Core::_main_loop, this);
  return true;
}

bool Core::stop() {
  _running = false;
  _thread.join();
  _logger->debug("Stopped");
  return true;
}

Node* Core::add_node(uint64_t id, sockaddr_in addr) {
  std::lock_guard<std::mutex> lock(_nodes_mutex);
  Node* node = new Node(id, addr);
  _nodes.insert({id, node});
  _nodes_by_addr.insert({(SockaddrHashable)addr, node});
  return node;
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

bool Core::queue_action(Action action) {
  std::lock_guard<std::mutex> lock(_action_mutex);
  _actions.push(action);
  _action_waiting.notify_one();  // Notify the waiting thread
  return true;
}

uint64_t Core::get_my_id() { return _my_id; }

KeyPair* Core::get_my_public_key() { return &_keys; }

void Core::_main_loop() {
  if (_running) {
    _logger->error("Start called but already running");
    return;
  }
  _running = true;
  _logger->debug("Started");

  while (_running) {
    std::unique_lock<std::mutex> lock(_action_mutex);
    if (_action_waiting.wait_for(lock, std::chrono::milliseconds(100), [this] { return !_actions.empty(); })) {
      while (!_actions.empty()) {
        Action action = _actions.front();
        _actions.pop();
        switch (action.type) {
          default:
            _logger->error("Unknown action type");
        }
      }
    }
  }

  _logger->debug("Stopping...");
}

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
