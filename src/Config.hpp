//
// Kapua Config class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#pragma once

#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "Kapua.hpp"
#include "Logger.hpp"

namespace Kapua {

class Config {
 public:
  Config(Logger* logger, std::string filename);
  ~Config();

  bool load();
  bool dump();

  enum class ParseResult { Success, InvalidFormat, InvalidUnit };

  // Config Parameters

  sockaddr_in server_address;  // server.ip4_address
  uint16_t server_port;        // server.port

  bool local_discovery_enable;              // local_discovery.emable
  sockaddr_in local_discovery_ip4_address;  // local_discovery.ip4_address
  uint16_t local_discovery_port;            // local_discovery.port
  uint64_t local_discovery_interval_ms;     // local_discovery.interval

  bool trackers_enable;                       // trackers.emable
  std::vector<std::string> trackers_servers;  // trackers.servers

  LogLevel_t logging_level;  // logging.level

 protected:
  Logger* _logger;
  std::string _filename;

  ParseResult parse_duration(const std::string& input, long long& milliseconds);
  ParseResult parse_ipv4(const std::string& input, in_addr* addr);
};

};  // namespace Kapua
