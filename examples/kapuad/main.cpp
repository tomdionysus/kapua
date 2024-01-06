#include <signal.h>

#include <chrono>
#include <iostream>
#include <thread>

#include "Core.hpp"
#include "Kapua.hpp"
#include "Logger.hpp"
#include "Protocol.hpp"
#include "UDPNetwork.hpp"
#include "RSA.hpp"

#include <fstream>

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
  Kapua::RSA rsa(&stdlog, &config);
  Kapua::Core core(&stdlog, &config, &rsa);

  if(!std::ifstream("private.pem").good()) {
    rsa.generate_rsa_key_pair("public.pem", "private.pem", 2048);
  }

  rsa.load_rsa_key_pair("public.pem", "private.pem", core._keys);

  Kapua::UDPNetwork local_discover(&stdlog, &config, &core, &rsa);

  if (!config.load_yaml("config.yaml")) {
    stdlog.error("Cannot load YAML config");
    return EXIT_FAILURE;
  };

  if (!config.load_cmd_line(ac, av)) {
    return EXIT_FAILURE;
  };

  stdlog.set_log_level(config.logging_level);

  if (!config.logging_disable_splash) {
    stdlog.raw("----------------------------");
    stdlog.raw("Kapua v" + Kapua::KAPUA_VERSION_STRING);
    stdlog.raw("----------------------------");
  }

  stdlog.info("Starting...");
  if (!core.start()) {
    stdlog.error("core start failed");
    return EXIT_FAILURE;
  }

  stdlog.debug("Starting...");
  if (!local_discover.start(ntohs(config.server_ip4_sockaddr.sin_port))) {
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

  if (!config.logging_disable_splash) {
    stdlog.raw("----------------------------");
  }
  return EXIT_SUCCESS;
}