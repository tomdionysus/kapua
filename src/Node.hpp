//
// Kapua Node class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#pragma once

#include <chrono>
#include <cstdint>
#include <vector>

#include "Node.hpp"
#include "SockaddrHashable.hpp"

namespace Kapua {

// CAVEAT: Do not instatiate Node, all instances are managed by the Core class.
class Node {
 public:
  Node(uint64_t pid) {
    id = pid;
    addr = sockaddr_in();
  }
  Node(uint64_t pid, sockaddr_in paddr) {
    id = pid;
    addr = paddr;
  }
  ~Node() {}

  void update_last_contact() { last_contact_time = std::chrono::steady_clock::now(); }

  SockaddrHashable addr;
  uint64_t id;

  std::chrono::time_point<std::chrono::steady_clock> last_contact_time;

 protected:
  std::vector<uint64_t> groups;
};

};  // namespace Kapua