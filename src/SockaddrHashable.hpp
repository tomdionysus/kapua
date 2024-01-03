//
// Kapua SockaddrHashable class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#pragma once

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <functional>

// Class that inherits from sockaddr_in and provides custom hashing
class SockaddrHashable : public sockaddr_in {
 public:
  SockaddrHashable() {
    sin_family = AF_INET;
    sin_port = 0;
    sin_addr.s_addr = INADDR_ANY;
  }

  SockaddrHashable(uint16_t port, uint32_t addr) {
    sin_family = AF_INET;
    sin_port = htons(port);
    sin_addr.s_addr = htonl(addr);
  }

  SockaddrHashable(sockaddr_in src) {
    sin_family = src.sin_family;
    sin_port = src.sin_port;
    sin_addr = src.sin_addr;
  }

  friend bool operator==(const SockaddrHashable& lhs, const SockaddrHashable& rhs) {
    return lhs.sin_port == rhs.sin_port && lhs.sin_addr.s_addr == rhs.sin_addr.s_addr;
  }

  friend bool operator!=(const SockaddrHashable& lhs, const SockaddrHashable& rhs) {
    return lhs.sin_port != rhs.sin_port || lhs.sin_addr.s_addr != rhs.sin_addr.s_addr;
  }

 private:
  // Helper function to hash individual elements
  template <typename T>
  std::size_t hash_combine(std::size_t seed, const T& v) const {
    std::hash<T> hasher;
    return seed ^ (hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
  }

  // Friend declaration for custom hash function
  friend struct SockaddrHashableer;
};

// Custom hash function
struct SockaddrHashableer {
  std::size_t operator()(const SockaddrHashable& sockAddrHash) const {
    std::size_t seed = 0;

    // Combine the hash of the IP address and the port
    seed = sockAddrHash.hash_combine(seed, sockAddrHash.sin_addr.s_addr);
    seed = sockAddrHash.hash_combine(seed, sockAddrHash.sin_port);

    return seed;
  }
};

// Specialization of std::hash for SockaddrHashable
namespace std {
template <>
struct hash<SockaddrHashable> {
  std::size_t operator()(const SockaddrHashable& sockAddrHash) const {
    SockaddrHashableer hasher;
    return hasher(sockAddrHash);
  }
};
}  // namespace std