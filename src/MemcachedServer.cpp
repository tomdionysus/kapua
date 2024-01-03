#include "MemcachedServer.hpp"

#include <iostream>

namespace Kapua {

MemcachedServer::MemcachedServer(Logger* logger, Config* config, Core* core) {
  _logger = new ScopedLogger("Settingsuration", logger);

  _config = config;
  _core = core;
  start();
}

MemcachedServer::~MemcachedServer() {
  stop();

  delete _logger;
}

void MemcachedServer::start() {}

void MemcachedServer::stop() {}

}  // namespace Kapua