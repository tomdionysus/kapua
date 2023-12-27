#include <signal.h>

#include <chrono>
#include <iostream>
#include <thread>

#include "Core.hpp"
#include "UDPNetwork.hpp"
#include "Logger.hpp"
#include "Protocol.hpp"

using namespace std;

Kapua::IOStreamLogger* stdlog;
Kapua::Config* config;
Kapua::Core* core;
Kapua::UDPNetwork* local_discover;

volatile bool running = true;
volatile bool stopping = false;

void signal_stop(int signum) {
  stdlog->info("SIGINT Recieved");
  if (!stopping) {
    running = false;
    stopping = true;

  } else {
    stdlog->warn("Hard shutdown!");
    exit(1);
  }
}

void start() {
  stdlog = new Kapua::IOStreamLogger(&cout, Kapua::LOG_LEVEL_DEBUG);
  config = new Kapua::Config(stdlog, "config.yaml");
  core = new Kapua::Core(stdlog, config);
  local_discover = new Kapua::UDPNetwork(stdlog, core);

  local_discover->start(KAPUA_PORT);
}

void stop() {
  local_discover->stop();

  delete local_discover;
  delete core;
  delete config;
}

int main() {
  stdlog = new Kapua::IOStreamLogger(&cout, Kapua::LOG_LEVEL_DEBUG);

  stdlog->debug("Starting...");
  start();
  stdlog->info("Started");

  signal(SIGINT, signal_stop);

  while (running) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  stdlog->debug("Stopping...");
  stop();
  stdlog->info("Stopped");

  delete stdlog;

  return EXIT_SUCCESS;
}