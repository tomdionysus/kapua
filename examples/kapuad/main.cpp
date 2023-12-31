#include <signal.h>

#include <chrono>
#include <iostream>
#include <thread>

#include "Kapua.hpp"
#include "Core.hpp"
#include "UDPNetwork.hpp"
#include "Logger.hpp"
#include "Protocol.hpp"

using namespace std;

volatile bool running = true;
volatile bool stopping = false;

void signal_stop(int signum) {
  if (!stopping) {
    running = false;
    stopping = true;

  } else {
    exit(1);
  }
}

int main(int ac, char** av) {
  Kapua::IOStreamLogger stdlog(&cout, Kapua::LOG_LEVEL_DEBUG);
  Kapua::Config config(&stdlog);
  Kapua::Core core(&stdlog, &config);
  Kapua::UDPNetwork local_discover(&stdlog, &core);

  stdlog.raw("----------------------------");
  stdlog.raw("Kapua v"+Kapua::KAPUA_VERSION_STRING);
  stdlog.raw("----------------------------");

  stdlog.info("Confguring...");

  if (!config.load_yaml("config.yaml")) {
    stdlog.error("Cannot load YAML config");
    return EXIT_FAILURE;
  };

  if(!config.load_cmd_line(ac,av)) {
    return EXIT_SUCCESS;
  };

  stdlog.info("Starting...");
  if(!core.start()) {
    stdlog.error("core start failed");
    return EXIT_FAILURE;
  }

  stdlog.debug("Starting...");
  if(!local_discover.start(ntohs(config.server_address.sin_port))) {
    stdlog.error("local discover start failed");
    return EXIT_FAILURE;
  }
  stdlog.info("Started");

  signal(SIGINT, signal_stop);

  while (running) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  stdlog.debug("Stopping...");
  local_discover.stop();
  stdlog.info("Stopped");
  stdlog.raw("----------------------------");

  return EXIT_SUCCESS;
}