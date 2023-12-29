//
// Kapua Config class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#include "Config.hpp"

#include <yaml-cpp/yaml.h>

#include "Logger.hpp"

using namespace std;

namespace Kapua {

Config::Config(Logger* logger, std::string filename) {
  _logger = new ScopedLogger("Configuration", logger);
  _filename = filename;

  // Defaults
  server_address.sin_family = AF_INET;
  inet_pton(AF_INET, "0.0.0.0", &server_address.sin_addr);
  server_address.sin_port = htons(KAPUA_DEFAULT_PORT);
}

Config::~Config() { delete _logger; }

bool Config::load() {
  _logger->info("Loading config file " + _filename);

  try {
    YAML::Node config = YAML::LoadFile(_filename);

    if (config.Type() != YAML::NodeType::Map) {
      _logger->error("Root YAML for config must be a map");
      return false;
    }

    // server
    if (!config["server"]) {
      _logger->error("config.server is required");
      return false;
    }

    // server.ip4_address
    if (config["server"]["ip4_address"]) {
      struct in_addr addr;
      if (parse_ipv4(config["server"]["ip4_address"].as<std::string>(), &server_address.sin_addr) != Config::ParseResult::Success) {
        _logger->error("server.address must be a valid IP address");
        return false;
      }
      _logger->debug("server.address = " + std::string(inet_ntoa(server_address.sin_addr)));
      server_address.sin_port = htons(KAPUA_DEFAULT_PORT);
    }

    // server.port
    if (config["server"]["port"]) {
      uint16_t port = config["server"]["port"].as<uint16_t>();
      _logger->debug("server.port = " + std::to_string(port));
      server_address.sin_port = htons(port);
    }

  } catch (const YAML::BadFile& e) {
    _logger->error("Cannot open file " + _filename + " (" + e.msg + ")");
    return false;
  } catch (const YAML::ParserException& e) {
    _logger->error("Cannot parse file " + _filename + " (" + e.msg + ")");
    return false;
  }
  return true;
}

bool Config::dump() {
  _logger->debug("Printing config");
  return true;
}

Config::ParseResult Config::parse_duration(const std::string& input, long long& milliseconds) {
  std::vector<std::pair<double, char>> components;  // Pairs of value and unit
  size_t pos = 0;
  size_t len = input.length();
  bool lastIsFraction = false;  // Track if the last quantity is a fraction
  bool isNegative = false;

  // Handle negative duration
  if (!input.empty() && input[pos] == '-') {
    isNegative = true;
    pos++;
  }

  char lastUnit = 'u';  // Start with the smallest unit for comparison

  while (pos < len) {
    double value = 0.0;
    char unit = '\0';
    bool hasFraction = false;      // Track if a fraction is present
    bool fractionStarted = false;  // Track if we are parsing the fractional part

    while (pos < len && (isdigit(input[pos]) || input[pos] == '.')) {
      if (input[pos] == '.') {
        if (hasFraction) {  // Second decimal point found - invalid format
          return ParseResult::InvalidFormat;
        }
        hasFraction = true;
        fractionStarted = true;
      } else {
        value = value * 10 + (input[pos] - '0');
        if (fractionStarted) {
          value /= 10;
        }
      }
      pos++;
    }

    // Parse the unit part
    if (pos < len) {
      unit = input[pos];
      pos++;
    }

    // Validate unit
    if ((unit != 'h' && unit != 'm' && unit != 's' && unit != 'u') || (unit == '\0' && pos < len)) {
      return ParseResult::InvalidUnit;
    }

    // Validate the order of units
    if ((unit == 'h' && lastUnit != 'u') || (unit == 'm' && lastUnit != 'u' && lastUnit != 'h') ||
        (unit == 's' && lastUnit != 'u' && lastUnit != 'h' && lastUnit != 'm') || (unit == 'u' && lastUnit != 'u')) {
      return ParseResult::InvalidFormat;
    }

    lastUnit = unit;  // Update the last seen unit
    components.emplace_back(value, unit);
    lastIsFraction = hasFraction;
  }

  // Ensure the last element is the only fraction
  for (size_t i = 0; i < components.size() - 1; ++i) {
    if (components[i].first != static_cast<long long>(components[i].first)) {
      return ParseResult::InvalidFormat;
    }
  }

  milliseconds = 0;
  for (const auto& component : components) {
    switch (component.second) {
      case 'h':
        milliseconds += static_cast<long long>(component.first * 3600000);
        break;
      case 'm':
        milliseconds += static_cast<long long>(component.first * 60000);
        break;
      case 's':
        milliseconds += static_cast<long long>(component.first * 1000);
        break;
      case 'u':
        milliseconds += static_cast<long long>(component.first);
        break;
    }
  }

  // Apply negative sign if needed
  if (isNegative) {
    milliseconds = -milliseconds;
  }

  return ParseResult::Success;
}

Config::ParseResult Config::parse_ipv4(const std::string& input, in_addr* addr) {
  if (!inet_pton(AF_INET, input.c_str(), addr)) return Config::ParseResult::InvalidFormat;
  return Config::ParseResult::Success;
}

Config::ParseResult Config::parse_log_level(const std::string& input, LogLevel_t* level) {
    std::string lowercaseInput = input;

    // Convert the input string to lowercase
    for (char& c : lowercaseInput) {
        c = std::tolower(c);
    }

    if (lowercaseInput == "error") {
        *level = LOG_LEVEL_ERROR;
        return ParseResult::Success;
    } else if (lowercaseInput == "warn" || lowercaseInput == "warning") {
        *level = LOG_LEVEL_WARN;
        return ParseResult::Success;
    } else if (lowercaseInput == "info") {
        *level = LOG_LEVEL_INFO;
        return ParseResult::Success;
    } else if (lowercaseInput == "debug") {
        *level = LOG_LEVEL_DEBUG;
        return ParseResult::Success;
    } else {
        return ParseResult::InvalidFormat;
    }
}

}  // namespace Kapua