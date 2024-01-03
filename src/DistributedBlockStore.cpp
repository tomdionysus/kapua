//
// Kapua DistributedBlockStore class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#include "DistributedBlockStore.hpp"

#include <algorithm>
#include <limits>

namespace Kapua {
DistributedBlockStore::DistributedBlockStore(uint64_t id, const std::vector<uint64_t>& virtualIds, uint64_t capacity) : nodeId(id) {
  for (auto vid : virtualIds) {
    ring.insert(vid);
    virtual_to_real[vid] = id;
  }
  node_capacities[id] = std::make_pair(capacity, 0);
}

void DistributedBlockStore::add_dbs_node(uint64_t id, const std::vector<uint64_t>& virtualIds, uint64_t capacity) {
  for (auto vid : virtualIds) {
    ring.insert(vid);
    virtual_to_real[vid] = id;
  }
  node_capacities[id] = std::make_pair(capacity, 0);
}

void DistributedBlockStore::remove_dbs_node(uint64_t id) {
  for (auto it = virtual_to_real.begin(); it != virtual_to_real.end();) {
    if (it->second == id) {
      ring.erase(it->first);
      it = virtual_to_real.erase(it);
    } else {
      ++it;
    }
  }
  node_capacities.erase(id);
}

void DistributedBlockStore::update_dbs_node_capacity(uint64_t id, uint64_t newCapacity) {
  if (node_capacities.find(id) != node_capacities.end()) {
    node_capacities[id].first = newCapacity;
  }
}

std::vector<uint64_t> DistributedBlockStore::get_dbs_nodes_for_block(uint64_t blockId) const {
  std::vector<uint64_t> nodes;
  if (ring.empty()) {
    return nodes;
  }

  auto it = ring.lower_bound(blockId);
  if (it == ring.end()) {
    it = ring.begin();
  }

  for (int i = 0; i < 5 && it != ring.end(); ++i) {
    uint64_t nodeId = virtual_to_real.at(*it);
    if (!check_node_overloaded(nodeId)) {
      nodes.push_back(*it);
    }
    ++it;
    if (it == ring.end()) {
      it = ring.begin();
    }
  }
  return nodes;
}

bool DistributedBlockStore::check_node_overloaded(uint64_t nodeId) const {
  auto nodeCapacity = node_capacities.find(nodeId);
  if (nodeCapacity != node_capacities.end()) {
    return nodeCapacity->second.second >= nodeCapacity->second.first;
  }
  return false;  // Assume not overloaded if node is not found
}
}  // namespace Kapua
