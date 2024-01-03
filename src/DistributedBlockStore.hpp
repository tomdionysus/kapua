//
// Kapua DistributedBlockStore class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#pragma once

#include <cstdint>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Kapua {

class DistributedBlockStore {
 public:
  DistributedBlockStore(uint64_t id, const std::vector<uint64_t>& virtualIds, uint64_t capacity);
  void add_dbs_node(uint64_t id, const std::vector<uint64_t>& virtualIds, uint64_t capacity);
  void remove_dbs_node(uint64_t id);
  void update_dbs_node_capacity(uint64_t id, uint64_t newCapacity);
  std::vector<uint64_t> get_dbs_nodes_for_block(uint64_t blockId) const;

 private:
  uint64_t nodeId;
  std::set<uint64_t> ring;                                                      // Ring structure
  std::unordered_map<uint64_t, uint64_t> virtual_to_real;                       // Map virtual to real node IDs
  std::unordered_map<uint64_t, std::pair<uint64_t, uint64_t>> node_capacities;  // Node capacities and usage

  bool check_node_overloaded(uint64_t nodeId) const;
};

}  // namespace Kapua
