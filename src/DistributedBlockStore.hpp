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
  void addNode(uint64_t id, const std::vector<uint64_t>& virtualIds, uint64_t capacity);
  void removeNode(uint64_t id);
  void updateNodeCapacity(uint64_t id, uint64_t newCapacity);
  std::vector<uint64_t> getNodesForBlock(uint64_t blockId) const;

 private:
  uint64_t nodeId;
  std::set<uint64_t> ring;                                                     // Ring structure
  std::unordered_map<uint64_t, uint64_t> virtualToReal;                        // Map virtual to real node IDs
  std::unordered_map<uint64_t, std::pair<uint64_t, uint64_t>> nodeCapacities;  // Node capacities and usage

  bool isNodeOverloaded(uint64_t nodeId) const;
};

}  // namespace Kapua
