#include "Config.hpp"

#include <yaml-cpp/yaml.h>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

#include "Logger.hpp"

using namespace std;
namespace po = boost::program_options;

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

  std::string source = "yaml";

  try {
    YAML::Node config = YAML::LoadFile(_filename);

    if (config.Type() != YAML::NodeType::Map) {
      _logger->error("Root YAML for config must be an object (YAML map)");
      return false;
    }

    // server.*
    if (config["server"]["id"] && !parse_server_id(source, config["server"]["id"].as<std::string>())) return false;
    if (config["server"]["ip4_address"] && !parse_server_address(source, config["server"]["ip4_address"].as<std::string>())) return false;
    if (config["server"]["port"] && !parse_server_port(source, config["server"]["port"].as<uint16_t>())) return false;

    // local_discovery.*
    if (config["local_discovery"]["enable"] && !parse_local_discovery_enable(source, config["local_discovery"]["enable"].as<std::string>())) return false;

  } catch (const YAML::BadFile& e) {
    _logger->error("Cannot open file " + _filename + " (" + e.msg + ")");
    return false;
  } catch (const YAML::ParserException& e) {
    _logger->error("Cannot parse file " + _filename + " (" + e.msg + ")");
    return false;
  } catch (exception& e) {
    _logger->error(std::string("Error while parsing configuration YAML: ") + e.what());
    return false;
  } catch (...) {
    _logger->error("Unknown error while parsing configuration YAML");
    return false;
  }
  return true;
}

bool Config::load_cmd_line(int ac, char** av) {

  std::string source = "cmd";

  try {
    po::options_description desc("Kapua v" + KAPUA_VERSION_STRING + "\nOptions:");
    desc.add_options()("help", "Print this help")("server.address", po::value<std::string>(), "server ipv4 address")(
        "server.port", po::value<uint16_t>(), "server ipv4 port")("local_discovery.enable", po::value<std::string>(), "enable UDP local discovery");

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
      cout << desc << "\n";
      return false;
    }

    // server_address
    if (vm.count("server.id") && !parse_server_id(source, vm["server.id"].as<std::string>())) return false;
    if (vm.count("server.address") && !parse_server_address(source, vm["server.address"].as<std::string>())) return false;
    if (vm.count("server.port") && !parse_server_port(source, vm["server.port"].as<uint16_t>())) return false;

    // local_discovery
    if (vm.count("local_discovery.enable") && !parse_local_discovery_enable(source, vm["local_discovery.enable"].as<std::string>())) return false;

  } catch (exception& e) {
    _logger->error(std::string("Error while parsing command line options: ") + e.what());
    return false;
  } catch (...) {
    _logger->error("Unknown error while parsing command line options");
    return false;
  }
  return true;
}

bool Config::dump() {
  _logger->debug("Printing config");
  return true;
}

Config::ParseResult Config::parse_duration(const std::string& input, int64_t& milliseconds) {
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
    if (components[i].first != static_cast<int64_t>(components[i].first)) {
      return ParseResult::InvalidFormat;
    }
  }

  milliseconds = 0;
  for (const auto& component : components) {
    switch (component.second) {
      case 'h':
        milliseconds += static_cast<int64_t>(component.first * 3600000);
        break;
      case 'm':
        milliseconds += static_cast<int64_t>(component.first * 60000);
        break;
      case 's':
        milliseconds += static_cast<int64_t>(component.first * 1000);
        break;
      case 'u':
        milliseconds += static_cast<int64_t>(component.first);
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

Config::ParseResult Config::parse_bool(const std::string& input, bool* result) {
  std::string parsedInput = input;
  boost::algorithm::to_lower(parsedInput);
  if (parsedInput == "true" || parsedInput == "t" || parsedInput == "yes") {
    *result = true;
  } else if (parsedInput == "false" || parsedInput == "f" || parsedInput == "no") {
    *result = false;
  } else {
    return ParseResult::InvalidFormat;
  }
  return ParseResult::Success;
}

Config::ParseResult Config::parse_log_level(const std::string& input, LogLevel_t* level) {
  std::string lowercaseInput = input;
  boost::algorithm::to_lower(lowercaseInput);

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

Config::ParseResult Config::parse_hex_uint64(const std::string& input, uint64_t& value) {
  std::string hexInput = input;

  // Remove "0x" prefix if present
  if (hexInput.size() >= 2 && hexInput.substr(0, 2) == "0x") {
    hexInput = hexInput.substr(2);
  }

  // Convert the input string to lowercase
  boost::algorithm::to_lower(hexInput);

  // Check if the string has exactly 16 hexadecimal characters
  if (hexInput.size() != 16 || !boost::algorithm::all(hexInput, ::isxdigit)) {
    return ParseResult::InvalidFormat;
  }

  try {
    // Use std::stoull to convert the hexadecimal string to uint64_t
    value = std::stoull(hexInput, nullptr, 16);
    return ParseResult::Success;
  } catch (const std::exception& e) {
    // Handle conversion error
    return ParseResult::InvalidFormat;
  }
}

bool Config::parse_server_id(const std::string& source, const std::string& input) {
  uint64_t id;

  if (parse_hex_uint64(input, id) != ParseResult::Success) {
  _logger->error("("+source+") server.id - invalid format: " + input + " - must be a valid 64-bit hex number");
    return false;
  }
  server_id = id;
  _logger->debug("("+source+") server.id = " + std::to_string(server_id));
  return true;
}

bool Config::parse_server_address(const std::string& source, const std::string& input) {
  if (parse_ipv4(input, &server_address.sin_addr) != ParseResult::Success) {
  _logger->error("("+source+") server.address - invalid format: " + input + " - must be a valid ipv4 address");
    return false;
  }
  _logger->debug("("+source+") server.address = " + input);
  server_address.sin_port = htons(KAPUA_DEFAULT_PORT);
  return true;
}

bool Config::parse_server_port(const std::string& source, const uint16_t port) {
  server_address.sin_port = htons(port);
  _logger->debug("("+source+") server.port = " + std::to_string(port));
  return true;
}

bool Config::parse_local_discovery_enable(const std::string& source, const std::string& input) {
  bool enable;
  if (parse_bool(input, &enable) != ParseResult::Success) {
  _logger->error("("+source+") local_discovery.enable - invalid format: " + input + " - must be 'true','t','yes','false','f','no'");
    return false;
  }
  local_discovery_enable = enable;
  _logger->debug("("+source+") local_discovery.enable = " + std::string(local_discovery_enable ? "true" : "false"));
  return true;
}

}  // namespace Kapua
