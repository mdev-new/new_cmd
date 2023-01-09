#include "win.hh"

#ifdef _WIN64

void setenv(char *name, char *value, bool overwrite) {
	SetEnvironmentVariable(name, value);
}

void unsetenv(char *name) {
	SetEnvironmentVariable(name, NULL);
}

char *realpath(char *rel, char *abs) {
	return _fullpath(abs, rel, PATH_MAX);
}

#endif