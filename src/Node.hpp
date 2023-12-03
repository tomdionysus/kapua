//
// Kapua core class
//
// Author: Tom Cully <mail@tomcully.com>
//
#pragma once

#include <cstdint>
#include <vector>

#include "Node.hpp"

namespace Kapua {

class Node {
 public:
  Node(uint64_t id);
  ~Node();

 protected:
  uint64_t _id;

  std::vector<uint64_t> groups;
};

};  // namespace Kapua