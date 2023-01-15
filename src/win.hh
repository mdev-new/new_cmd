#pragma once
#ifdef _WIN64

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <conio.h>

void setenv(char *name, char *value, bool overwrite);
void unsetenv(char *name);
char *realpath(char *rel, char *abs);

#else
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#endif
