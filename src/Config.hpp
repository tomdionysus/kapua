//
// Kapua Config class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#pragma once

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
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
  Config(Logger* logger);
  ~Config();

  bool load_yaml(std::string filename);
  bool load_cmd_line(int ac, char** av);

  bool dump();

  enum class ParseResult { Success, InvalidFormat, InvalidUnit };

  // Config Parameters
  uint64_t server_id;               // server.id
  sockaddr_in server_ip4_sockaddr;  // server.ip4_address
  uint16_t server_port;             // server.port

  bool local_discovery_enable;              // local_discovery.emable
  sockaddr_in local_discovery_ip4_address;  // local_discovery.ip4_address
  uint16_t local_discovery_port;            // local_discovery.port
  int32_t local_discovery_interval_ms;      // local_discovery.interval

  bool trackers_enable;                       // trackers.emable
  std::vector<std::string> trackers_servers;  // trackers.servers

  bool memcached_enable;                     // memcached.enable
  sockaddr_in memcached_ip4_sockaddr;        // server.ip4_address
  bool memcached_extensions;                 // memcached.extensions
  uint16_t memcached_connection_limit;       // memcached.connection_limit
  uint32_t memcached_inactivity_timeout_ms;  // memcached.inactivity_timeout

  LogLevel_t logging_level;     // logging.level
  bool logging_disable_splash;  // logging.disable_splash

 protected:
  Logger* _logger;

  // Generic Parsers
  bool parse_bool(const std::string& source, const std::string& name, const std::string& input, bool* result);
  bool parse_duration(const std::string& source, const std::string& name, const std::string& input, bool allowNegative, int32_t* milliseconds);
  bool parse_ipv4(const std::string& source, const std::string& name, const std::string& input, in_addr* addr);
  bool parse_port(const std::string& source, const std::string& name, const std::string& input, uint16_t* port);
  bool parse_log_level(const std::string& source, const std::string& name, const std::string& input, LogLevel_t* level);
  bool parse_hex_uint64(const std::string& source, const std::string& name, const std::string& input, uint64_t* value);

  bool parse_uint16(const std::string& source, const std::string& name, const std::string& input, uint16_t* value);
};

};  // namespace Kapua
