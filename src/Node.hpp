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
#include "RSA.hpp"
#include "SockaddrHashable.hpp"

namespace Kapua {

// CAVEAT: Do not instatiate Node, all instances are managed by the Core class.
class Node {
 public:
  enum class State {
    Initialised,
    KeyExchange,
    Handshake,
    CheckEncryption,
    Connected,
  };

  Node(uint64_t pid) {
    id = pid;
    addr = sockaddr_in();
    state = Node::State::Initialised;
  }
  Node(uint64_t pid, sockaddr_in paddr) {
    id = pid;
    addr = paddr;
    state = Node::State::Initialised;
  }
  ~Node() {}

  void update_last_contact() { last_contact_time = std::chrono::steady_clock::now(); }

  SockaddrHashable addr;
  uint64_t id;
  State state;

  KeyPair keys;

  AESContext aes_context_tx;
  AESContext aes_context_rx;

  std::chrono::time_point<std::chrono::steady_clock> last_contact_time;

 protected:
  std::vector<uint64_t> groups;
};

};  // namespace Kapua