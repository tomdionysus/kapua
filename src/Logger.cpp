#include "Logger.hpp"

#include <ctime>

using namespace std;

namespace Kapua {

void IOStreamLogger ::debug(std::string log) {
  cout << _getTimeStr() << " [DEBUG] " << log << "\n";
}

void IOStreamLogger ::info(std::string log) {
  cout << _getTimeStr() << " [INFO ] " << log << "\n";
}

void IOStreamLogger ::warn(std::string log) {
  cout << _getTimeStr() << " [WARN ] " << log << "\n";
}

void IOStreamLogger ::error(std::string log) {
  cerr << _getTimeStr() << " [ERROR] " << log << "\n";
}

std::string IOStreamLogger ::_getTimeStr() {
  time_t now;
  time(&now);
  char buf[sizeof "2011-10-08T07:07:09Z"];
  strftime(buf, sizeof buf, "%FT%TZ", gmtime(&now));
  return std::string(buf);
}

ScopedLogger ::ScopedLogger(std::string prefix, Logger* logger) {
  _logger = logger;
  _prefix = prefix;
}

void ScopedLogger ::debug(std::string log) {
  _logger->debug("(" + _prefix + ") " + log);
}

void ScopedLogger ::info(std::string log) {
  _logger->info("(" + _prefix + ") " + log);
}

void ScopedLogger ::warn(std::string log) {
  _logger->warn("(" + _prefix + ") " + log);
}

void ScopedLogger ::error(std::string log) {
  _logger->error("(" + _prefix + ") " + log);
}

}  // namespace Kapua