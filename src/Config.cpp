#include "Config.hpp"

#include <yaml-cpp/yaml.h>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

#include "Logger.hpp"
#include "Util.hpp"

using namespace std;
namespace po = boost::program_options;

namespace Kapua {

Config::Config(Logger* logger) {
  _logger = new ScopedLogger("Configuration", logger);

  // Defaults
  server_ip4_sockaddr.sin_family = AF_INET;
  inet_pton(AF_INET, "0.0.0.0", &server_ip4_sockaddr.sin_addr);
  server_ip4_sockaddr.sin_port = htons(KAPUA_DEFAULT_PORT);
}

Config::~Config() { delete _logger; }

bool Config::load_yaml(std::string filename) {
  const std::string source = "yaml";

  try {
    YAML::Node config = YAML::LoadFile(filename);

    if (config.Type() != YAML::NodeType::Map) {
      _logger->error("Root YAML for config must be an object (YAML map)");
      return false;
    }

    bool ok = true;

    // logging
    if (config["logging"]["level"]) ok &= parse_log_level(source, "logging.level", config["logging"]["level"].as<std::string>(), &logging_level);
    if (ok) {
      // Special case. The logging level applies immediately.
      _logger->set_log_level(logging_level);
    }

    // server.*
    if (config["server"]["id"]) ok &= parse_hex_uint64(source, "server.id", config["server"]["id"].as<std::string>(), &server_id);
    if (config["server"]["ip4_address"])
      ok &= parse_ipv4(source, "server.ip4_address", config["server"]["ip4_address"].as<std::string>(), &server_ip4_sockaddr.sin_addr);
    if (config["server"]["port"]) ok &= parse_port(source, "server.port", config["server"]["port"].as<std::string>(), &server_ip4_sockaddr.sin_port);

    // local_discovery.*
    if (config["local_discovery"]["enable"])
      ok &= parse_bool(source, "local_discovery.enable", config["local_discovery"]["enable"].as<std::string>(), &local_discovery_enable);
    if (config["local_discovery"]["interval"])
      ok &= parse_duration(source, "local_discovery.interval", config["local_discovery"]["interval"].as<std::string>(), false, &local_discovery_interval_ms);

    // memcache
    if (config["memcache"]["enable"]) ok &= parse_bool(source, "memcache.enable", config["memcache"]["enable"].as<std::string>(), &memcached_enable);
    if (config["memcache"]["port"]) ok &= parse_port(source, "memcache.port", config["memcache"]["port"].as<std::string>(), &memcached_ip4_sockaddr.sin_port);

    if (!ok) {
      _logger->error(std::string("Errors while parsing parsing configuration YAML"));
      return false;
    }

  } catch (const YAML::BadFile& e) {
    _logger->error("Cannot open file " + filename + " (" + e.msg + ")");
    return false;
  } catch (const YAML::ParserException& e) {
    _logger->error("Cannot parse file " + filename + " (" + e.msg + ")");
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
  const std::string source = "cmd";

  try {
    po::options_description desc("Kapua v" + KAPUA_VERSION_STRING + "\nOptions:");

    // clang-format off
    
    desc.add_options()
      ("help", "Print this help")
      ("server.id", po::value<std::string>(), "server id, 64-bit hex [0x123456789abcdef0]")
      ("server.ip4_address", po::value<std::string>(), "server ipv4 address [x.x.x.x]")
      ("server.port", po::value<uint16_t>(), "server ipv4 port [0-65535]")
      ("local_discovery.enable", po::value<std::string>(), "enable UDP local discovery [true,false]")
      ("logging.level", po::value<std::string>(), "set the logging level [debug,info,warn,error]");

    // clang-format on

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
      cout << desc << "\n";
      return false;
    }

    bool ok = true;

    // logging
    if (vm.count("logging.level")) ok &= parse_log_level(source, "logging.level", vm["logging.level"].as<std::string>(), &logging_level);
    if (ok) {
      // Special case. The logging level applies immediately.
      _logger->set_log_level(logging_level);
    }

    // server_ip4_sockaddr
    if (vm.count("server.id")) ok &= parse_hex_uint64(source, "server.id", vm["server.id"].as<std::string>(), &server_id);
    if (vm.count("server.ip4_address"))
      ok &= parse_ipv4(source, "server.ip4_address", vm["server.ip4_address"].as<std::string>(), &server_ip4_sockaddr.sin_addr);
    if (vm.count("server.port")) ok &= parse_port(source, "server.port", vm["server.port"].as<std::string>(), &server_ip4_sockaddr.sin_port);

    // local_discovery
    if (vm.count("local_discovery.enable"))
      ok &= parse_bool(source, "local_discovery.enable", vm["local_discovery.enable"].as<std::string>(), &local_discovery_enable);
    if (vm.count("local_discovery.interval"))
      ok &= parse_duration(source, "local_discovery.interval", vm["local_discovery.interval"].as<std::string>(), false, &local_discovery_interval_ms);

    // memcache
    if (vm.count("memcache.enable")) ok &= parse_bool(source, "memcache.enable", vm["memcache.enable"].as<std::string>(), &memcached_enable);
    if (vm.count("memcache.ip4_address"))
      ok &= parse_ipv4(source, "memcache.ip4_address", vm["memcache.ip4_address"].as<std::string>(), &memcached_ip4_sockaddr.sin_addr);
    if (vm.count("memcache.port")) ok &= parse_port(source, "memcache.port", vm["memcache.port"].as<std::string>(), &memcached_ip4_sockaddr.sin_port);

    if (!ok) {
      _logger->error(std::string("Errors while parsing command line options"));
      return false;
    }

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

bool Config::parse_duration(const std::string& source, const std::string& name, const std::string& input, bool allowNegative, int32_t* ms) {
  enum Mode { Start, Digit, Unit };
  bool isNegative;
  std::string buffer;

  int32_t res = 0;

  Mode mode = Mode::Start;

  for (char c : input) {
    // if (c == ' ' && mode != Mode::Digit) continue;

    if (mode == Mode::Start) {
      if (c == '-') {
        if (!allowNegative) {
          _logger->error("(" + source + ") " + name + " - invalid format: " + input + " - duration cannot be negative");
          return false;
        }
        isNegative = true;
      }
      if (!std::isdigit(c)) {
        _logger->error("(" + source + ") " + name + " - invalid format: " + input + " - (bad start) must be [d]h[d]m[d]s where [d] is an integer");
      }
      buffer.push_back(c);
      mode = Mode::Digit;
    } else if (mode == Mode::Digit) {
      if ((c >= '0') && (c <= '9')) {
        // Add it to the buffer
        buffer.push_back(c);
        continue;
      }

      // Bad format if any character is first (buffer is empty)
      if (buffer.length() == 0) {
        _logger->error("(" + source + ") " + name + " - invalid format: " + input + " - (no buffer) must be [d]h[d]m[d]s where [d] is an integer");
        return false;
      }

      // Process formats...
      if (c == 'h') {
        res += (std::stoi(buffer) * 60 * 60 * 1000);
        buffer.clear();
      } else if (c == 'm') {
        res += (std::stoi(buffer) * 60 * 1000);
        buffer.clear();
      } else if (c == 's') {
        res += (std::stoi(buffer) * 1000);
        buffer.clear();
      } else {
        // Bad format if not recognised
        _logger->error("(" + source + ") " + name + " - invalid format: " + input + " - unit not recognised, must be 'h','m','s'");
        return false;
      }
    }
  }

  *ms = res;

  _logger->debug("(" + source + ") " + name + " = " + std::to_string(res) + "ms");

  return true;
}

bool Config::parse_ipv4(const std::string& source, const std::string& name, const std::string& input, in_addr* addr) {
  if (!inet_pton(AF_INET, input.c_str(), addr)) {
    _logger->error("(" + source + ") " + name + " " + input + " - must be a valid ipv4 address");
    return false;
  }

  _logger->debug("(" + source + ") " + name + " = " + input);
  return true;
}

bool Config::parse_bool(const std::string& source, const std::string& name, const std::string& input, bool* result) {
  std::string parsedInput = input;
  boost::algorithm::to_lower(parsedInput);
  if (parsedInput == "true" || parsedInput == "t" || parsedInput == "yes") {
    *result = true;
  } else if (parsedInput == "false" || parsedInput == "f" || parsedInput == "no") {
    *result = false;
  } else {
    _logger->error("(" + source + ") " + name + " - invalid format: " + input + " - must be 'true','t','yes','false','f','no'");
    return false;
  }
  _logger->debug("(" + source + ") " + name + " = " + std::string(*result ? "true" : "false"));
  return true;
}

bool Config::parse_log_level(const std::string& source, const std::string& name, const std::string& input, LogLevel_t* level) {
  std::string lowercaseInput = input;
  boost::algorithm::to_lower(lowercaseInput);

  if (lowercaseInput == "error") {
    *level = LOG_LEVEL_ERROR;
  } else if (lowercaseInput == "warn" || lowercaseInput == "warning") {
    *level = LOG_LEVEL_WARN;
  } else if (lowercaseInput == "info") {
    *level = LOG_LEVEL_INFO;
  } else if (lowercaseInput == "debug") {
    *level = LOG_LEVEL_DEBUG;
  } else {
    _logger->error("(" + source + ") " + name + " - invalid format: " + input + " - must be 'true','t','yes','false','f','no'");
    return true;
  }

  return true;
}

bool Config::parse_hex_uint64(const std::string& source, const std::string& name, const std::string& input, uint64_t* value) {
  std::string hexInput = input;

  // Remove "0x" prefix if present
  if (hexInput.size() >= 2 && hexInput.substr(0, 2) == "0x") {
    hexInput = hexInput.substr(2);
  }

  // Convert the input string to lowercase
  boost::algorithm::to_lower(hexInput);

  // Check if the string has exactly 16 hexadecimal characters
  if (hexInput.size() != 16 || !boost::algorithm::all(hexInput, ::isxdigit)) {
    _logger->error("(" + source + ") " + name + " - invalid format: " + input + " - must be a valid 64-bit hex number");
    return false;
  }

  try {
    // Use std::stoull to convert the hexadecimal string to uint64_t
    *value = std::stoull(hexInput, nullptr, 16);
  } catch (const std::exception& e) {
    _logger->error("(" + source + ") " + name + " - invalid format: " + input + " - must be a valid 64-bit hex number");
    return false;
  }

  _logger->debug("(" + source + ") " + name + " = 0x" + Util::to_hex64_str(*value));
  return true;
}

bool Config::parse_uint16(const std::string& source, const std::string& name, const std::string& input, uint16_t* value) {
  int num;
  try {
    // Convert string to int
    num = std::stoi(input);

    // Cast to uint16_t
    *value = static_cast<uint16_t>(num);
  } catch (const std::invalid_argument& ia) {
    _logger->error("(" + source + ") " + name + " - invalid format: " + input + " - must be a valid 16-bit number");
    return false;
  } catch (const std::out_of_range& oor) {
    _logger->error("(" + source + ") " + name + " - invalid format: " + input + " - must be 0-65535");
    return false;
  }

  _logger->debug("(" + source + ") " + name + " = " + std::to_string(num));
  return true;
}

bool Config::parse_port(const std::string& source, const std::string& name, const std::string& input, uint16_t* port) {
  uint16_t pPort;
  if (!parse_uint16(source, name, input, &pPort)) return false;
  *port = htons(pPort);
  return true;
}

}  // namespace Kapua
