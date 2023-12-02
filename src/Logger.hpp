//
// Kapua core class
//
// Author: Tom Cully <mail@tomcully.com>
//

#pragma once

#include <iostream>
#include <string>

namespace Kapua {

enum LogLevel_t {
  LOG_LEVEL_ERROR = 0,
  LOG_LEVEL_WARN = 1,
  LOG_LEVEL_INFO = 2,
  LOG_LEVEL_DEBUG = 3,
};

class Logger {
 public:
  virtual void debug(std::string log) = 0;
  virtual void info(std::string log) = 0;
  virtual void warn(std::string log) = 0;
  virtual void error(std::string log) = 0;
};

class IOStreamLogger : public Logger {
 public:
  virtual void debug(std::string log) override;
  virtual void info(std::string log) override;
  virtual void warn(std::string log) override;
  virtual void error(std::string log) override;

 private:
  std::string _getTimeStr();
};

class ScopedLogger : public Logger {
 public:
  ScopedLogger(std::string prefix, Logger* logger);

  virtual void debug(std::string log) override;
  virtual void info(std::string log) override;
  virtual void warn(std::string log) override;
  virtual void error(std::string log) override;

 protected:
  std::string _prefix;
  Logger* _logger;
};
};  // namespace Kapua