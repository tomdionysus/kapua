#include "Core.hpp"

#include "Logger.hpp"

using namespace std;

namespace Kapua {
Core ::Core(Logger* logger) {
  _logger = logger;
  _logger->info("Kapura startup");
}

Core ::~Core() { _logger->info("Kapura shutdown"); }

void Core::addNode(uint64_t id, Node* node) {
  std::lock_guard<std::mutex> lock(_nodes_mutex);
  _nodes.insert({id, node});
}

void Core::removeNode(uint64_t id) {
  std::lock_guard<std::mutex> lock(_nodes_mutex);
  _nodes.erase(id);
}

Node* Core::findNode(uint64_t id) {
  std::lock_guard<std::mutex> lock(_nodes_mutex);
  auto search = _nodes.find(id);
  if (search != _nodes.end()) return search->second;
  return nullptr;
}

void getVersion(Version_t* version) {
  version->major = KAPUA_VERSION_MAJOR;
  version->minor = KAPUA_VERSION_MINOR;
  version->patch = KAPUA_VERSION_PATCH;
}

}  // namespace Kapua
