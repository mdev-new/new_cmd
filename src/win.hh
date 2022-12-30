#pragma once
#ifdef _WIN64

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

void setenv(char *name, char *value, bool overwrite) {
	SetEnvironmentVariable(name, value);
}

void unsetenv(char *name) {
	SetEnvironmentVariable(name, NULL);
}

#endif
