#include <iostream>

#include "Core.hpp"
#include "Logger.hpp"

using namespace std;

int main() {
	Kapua::IOStreamLogger stdlog(&cout, Kapua::LOG_LEVEL_DEBUG);

	Kapua::ScopedLogger logger("Core", &stdlog);

	Kapua::Config config(&logger, "config.yaml");

	Kapua::Core core(&logger, &config);

	cout << "Kapua Example\n";
}