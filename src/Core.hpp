//
// Kapua Core class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#pragma once

#define KAPUA_VERSION_MAJOR 0
#define KAPUA_VERSION_MINOR 0
#define KAPUA_VERSION_PATCH 1

#include <atomic>
#include <boost/thread.hpp>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <random>
#include <unordered_map>
#include <vector>

#include "Actions.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include "Node.hpp"
#include "RSA.hpp"
#include "SockaddrHashable.hpp"

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace Kapua {

typedef struct Version {
  uint8_t major;
  uint8_t minor;
  uint8_t patch;
} Version_t;

class Core {
 public:
  Core(Logger* logger, Config* config, RSA* rsa);
  ~Core();

  bool start();
  bool stop();

  Node* add_node(uint64_t id, sockaddr_in addr);
  void remove_node(uint64_t id);
  Node* find_node(uint64_t id);
  Node* find_node(sockaddr_in addr);

  bool queue_action(Action action);

  void action_request_public_key(Action action);

  uint64_t get_my_id();
  KeyPair* get_my_public_key();

  void get_version(Version_t* version);
  
  KeyPair _keys;

 protected:
  Logger* _logger;
  Config* _config;

  RSA* _rsa;

  uint64_t _my_id;

  std::string _config_filename;

  std::unordered_map<uint64_t, Node*> _nodes;
  std::unordered_map<SockaddrHashable, Node*> _nodes_by_addr;
  std::mutex _nodes_mutex;

  std::unordered_map<uint64_t, std::vector<Node>> _groups;
  std::mutex _groups_mutex;

  std::queue<Action> _actions;
  std::condition_variable _action_waiting;
  std::mutex _action_mutex;

  boost::thread _thread;

  uint64_t _get_random_id();

  void _main_loop();

  std::atomic<bool> _running;
};

};  // namespace Kapua