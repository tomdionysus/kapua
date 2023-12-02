#include "Core.hpp"

#include "Logger.hpp"

using namespace std;

namespace Kapua {
Core ::Core(Logger& logger) : _logger(logger) {
  _logger.info("Kapura startup");
}

Core ::~Core() { _logger.info("Kapura shutdown"); }
}  // namespace Kapua