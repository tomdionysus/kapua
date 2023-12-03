#include <stdio.h>

#include "Core.hpp"
#include "Logger.hpp"

int main() {
	Kapua::IOStreamLogger stdlog;

	Kapua::ScopedLogger logger("Core", &stdlog);

	Kapua::Core core(&logger);

	printf("Kapua Example\n");
}