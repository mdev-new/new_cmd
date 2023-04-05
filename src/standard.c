#include "standard.h"

void stdcall std_Sleep(uint32_t ms) {
#ifdef _WIN64
	Sleep(ms);
#else
	usleep(ms * 1000);
#endif
	return;
}

void stdcall std_setenv(char *name, char *value) {
#ifdef _WIN64
	SetEnvironmentVariable(name, value);
#else
	setenv(name, value, true);
#endif
}

void stdcall std_unsetenv(char *name) {
#ifdef _WIN64
	SetEnvironmentVariable(name, NULL);
#else
	unsetenv(name);
#endif
}

char* stdcall std_realpath(char *abs, char *rel) {
#ifdef _WIN64
	return _fullpath(abs, rel, PATH_MAX);
#else
	return realpath(rel, abs);
#endif
}

