//
// Kapua MemcachedServer class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#pragma once

#include <memory>
#include <thread>
#include <vector>

#include "Config.hpp"
#include "Core.hpp"
#include "Logger.hpp"

namespace Kapua {

class MemcachedServer {
 public:
  MemcachedServer(Logger* logger, Config* config, Core* core);
  ~MemcachedServer();

  void start();
  void stop();

 private:
  Logger* _logger;
  Config* _config;
  Core* _core;
};

}  // namespace Kapua
