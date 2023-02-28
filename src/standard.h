#pragma once
#ifdef _WIN64

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <conio.h>
#include <direct.h>

#define acess(x, y) _acess(x, y)
#define setenv(name, value, overwrite) SetEnvironmentVariable(name, value)
#define unsetenv(name) SetEnvironmentVariable(name, NULL)
#define realpath(abs, rel) _fullpath(abs, rel, PATH_MAX)

#define timersub(a, b, result)                                                \
  do {                                                                        \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;                             \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;                          \
    if ((result)->tv_usec < 0) {                                              \
      --(result)->tv_sec;                                                     \
      (result)->tv_usec += 1000000;                                           \
    }                                                                         \
  } while (0)

#else

#define Sleep(t) usleep(t * 1000)

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <alloca.h>

#endif
