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

#include "Config.hpp"
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
  Core(Logger* logger, Config* config);
  ~Core();

  void add_node(uint64_t id, Node* node);
  void remove_node(uint64_t id);
  Node* find_node(uint64_t id);

  void get_version(Version_t* version);

 protected:
  Logger* _logger;
  Config* _config;

  std::unordered_map<uint64_t, Node*> _nodes;
  std::mutex _nodes_mutex;

  std::unordered_map<uint64_t, std::vector<Node>> _groups;
  std::mutex _groups_mutex;
};

};  // namespace Kapua