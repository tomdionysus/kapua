//
// Kapua Logger classes
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023 
//
#include "Logger.hpp"

using namespace std;

namespace Kapua {

IOStreamLogger ::IOStreamLogger(std::ostream* stream, LogLevel_t level) {
  _output_stream = stream;
  _log_level = level;
}

void IOStreamLogger ::debug(std::string log) {
  if (_log_level < 3) return;
  std::lock_guard<std::mutex> lock(_logging_mutex);
  cout << _getTimeStr() << " [DEBUG] " << log << "\n";
}

void IOStreamLogger ::info(std::string log) {
  if (_log_level < 2) return;
  std::lock_guard<std::mutex> lock(_logging_mutex);
  cout << _getTimeStr() << " [INFO ] " << log << "\n";
}

void IOStreamLogger ::warn(std::string log) {
  if (_log_level < 1) return;
  std::lock_guard<std::mutex> lock(_logging_mutex);
  cout << _getTimeStr() << " [WARN ] " << log << "\n";
}

void IOStreamLogger ::error(std::string log) {
  std::lock_guard<std::mutex> lock(_logging_mutex);
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

ScopedLogger::ScopedLogger(std::string prefix, Logger* logger, LogLevel_t level) {
  _logger = logger;
  _prefix = prefix;
  _log_level = level;
  _using_log_level = true;
}

void ScopedLogger ::debug(std::string log) {
  if (_using_log_level && _log_level < 3) return;
  _logger->debug("(" + _prefix + ") " + log);
}

void ScopedLogger ::info(std::string log) {
  if (_using_log_level && _log_level < 2) return;
  _logger->info("(" + _prefix + ") " + log);
}

void ScopedLogger ::warn(std::string log) {
  if (_using_log_level && _log_level < 1) return;
  _logger->warn("(" + _prefix + ") " + log);
}

void ScopedLogger ::error(std::string log) { _logger->error("(" + _prefix + ") " + log); }

}  // namespace Kapua