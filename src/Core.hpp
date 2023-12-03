//
// Kapua core class
//
// Author: Tom Cully <mail@tomcully.com>
//
#pragma once

#define KAPUA_VERSION_MAJOR 0
#define KAPUA_VERSION_MINOR 0
#define KAPUA_VERSION_PATCH 1

#include <mutex>
#include <unordered_map>
#include <vector>

#include "Logger.hpp"
#include "Node.hpp"

namespace Kapua {

typedef struct Version {
  uint8_t major;
  uint8_t minor;
  uint8_t patch;
} Version_t;

class Core {
 public:
  Core(Logger* logger);
  ~Core();

  void addNode(uint64_t id, Node* node);
  void removeNode(uint64_t id);
  Node* findNode(uint64_t id);

  void getVersion(Version_t* version);

 protected:
  Logger* _logger;

  std::unordered_map<uint64_t, Node*> _nodes;
  std::mutex _nodes_mutex;

  std::unordered_map<uint64_t, std::vector<Node>> _groups;
  std::mutex _groups_mutex;
};

};  // namespace Kapua