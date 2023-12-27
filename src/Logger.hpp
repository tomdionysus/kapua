//
// Kapua Logger classes
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#pragma once

#include <ctime>
#include <iostream>
#include <string>

namespace Kapua {

enum LogLevel_t : uint8_t {
  LOG_LEVEL_ERROR = 0,
  LOG_LEVEL_WARN = 1,
  LOG_LEVEL_INFO = 2,
  LOG_LEVEL_DEBUG = 3,
};

class Logger {
 public:
  virtual ~Logger() {}
  virtual void debug(std::string log) = 0;
  virtual void info(std::string log) = 0;
  virtual void warn(std::string log) = 0;
  virtual void error(std::string log) = 0;
};

class IOStreamLogger : public Logger {
 public:
  IOStreamLogger(std::ostream* stream, LogLevel_t level);
  ~IOStreamLogger() {}

  virtual void debug(std::string log) override;
  virtual void info(std::string log) override;
  virtual void warn(std::string log) override;
  virtual void error(std::string log) override;

 private:
  std::ostream* _output_stream;
  LogLevel_t _log_level;

  std::mutex _logging_mutex;

  std::string _getTimeStr();
};

class ScopedLogger : public Logger {
 public:
  ScopedLogger(std::string prefix, Logger* logger);
  ScopedLogger(std::string prefix, Logger* logger, LogLevel_t level);
  ~ScopedLogger() {}

  virtual void debug(std::string log) override;
  virtual void info(std::string log) override;
  virtual void warn(std::string log) override;
  virtual void error(std::string log) override;

 protected:
  std::string _prefix;
  Logger* _logger;

  LogLevel_t _log_level;
  bool _using_log_level = false;
};
};  // namespace Kapua