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
    virtualToReal[vid] = id;
  }
  nodeCapacities[id] = std::make_pair(capacity, 0);
}

void DistributedBlockStore::addNode(uint64_t id, const std::vector<uint64_t>& virtualIds, uint64_t capacity) {
  for (auto vid : virtualIds) {
    ring.insert(vid);
    virtualToReal[vid] = id;
  }
  nodeCapacities[id] = std::make_pair(capacity, 0);
}

void DistributedBlockStore::removeNode(uint64_t id) {
  for (auto it = virtualToReal.begin(); it != virtualToReal.end();) {
    if (it->second == id) {
      ring.erase(it->first);
      it = virtualToReal.erase(it);
    } else {
      ++it;
    }
  }
  nodeCapacities.erase(id);
}

void DistributedBlockStore::updateNodeCapacity(uint64_t id, uint64_t newCapacity) {
  if (nodeCapacities.find(id) != nodeCapacities.end()) {
    nodeCapacities[id].first = newCapacity;
  }
}

std::vector<uint64_t> DistributedBlockStore::getNodesForBlock(uint64_t blockId) const {
  std::vector<uint64_t> nodes;
  if (ring.empty()) {
    return nodes;
  }

  auto it = ring.lower_bound(blockId);
  if (it == ring.end()) {
    it = ring.begin();
  }

  for (int i = 0; i < 5 && it != ring.end(); ++i) {
    uint64_t nodeId = virtualToReal.at(*it);
    if (!isNodeOverloaded(nodeId)) {
      nodes.push_back(*it);
    }
    ++it;
    if (it == ring.end()) {
      it = ring.begin();
    }
  }
  return nodes;
}

bool DistributedBlockStore::isNodeOverloaded(uint64_t nodeId) const {
  auto nodeCapacity = nodeCapacities.find(nodeId);
  if (nodeCapacity != nodeCapacities.end()) {
    return nodeCapacity->second.second >= nodeCapacity->second.first;
  }
  return false;  // Assume not overloaded if node is not found
}
}  // namespace Kapua
