#include <iostream>
#include <thread>
#include <chrono>
#include <signal.h>

#include "Core.hpp"
#include "Logger.hpp"

using namespace std;

Kapua::IOStreamLogger* stdlog;
Kapua::ScopedLogger* logger;
Kapua::Config* config;
Kapua::Core* core;

volatile bool running = true;

void signal_callback_handler(int signum) {
   cout << "Caught signal " << signum << endl;
   // Terminate program
   running = false;
}

void start() {
	stdlog = new Kapua::IOStreamLogger(&cout, Kapua::LOG_LEVEL_DEBUG);
	logger = new Kapua::ScopedLogger("Core", stdlog);
	config = new Kapua::Config(logger, "config.yaml");
	core = new Kapua::Core(logger, config);
}

void stop() {
	delete core;
	delete config;

}

int main() {

stdlog = new Kapua::IOStreamLogger(&cout, Kapua::LOG_LEVEL_DEBUG);
	logger = new Kapua::ScopedLogger("Core", stdlog);

	stdlog->debug("Starting...");
	start();
	stdlog->info("Started");

	signal(SIGINT, signal_callback_handler);

   while(running){
      std::this_thread::sleep_for (std::chrono::seconds(1));
   }

	stdlog->debug("Stopping...");
   stop();
	stdlog->info("Stopped");

	delete logger;
	delete stdlog;
	
   return EXIT_SUCCESS;
}