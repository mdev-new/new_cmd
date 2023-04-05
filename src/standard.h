#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// these _should_ exist everywhere.
// even os msvc.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifndef _MSC_VER
#	define stdcall __attribute__((stdcall))
#else
#	define stdcall __stdcall
#endif

void stdcall std_Sleep(uint32_t ms);
void stdcall std_setenv(char *name, char *value);
void stdcall std_unsetenv(char *name);
char* stdcall std_realpath(char *abs, char *rel);

#ifdef _WIN64 // compatibility for unix syscalls and functions

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <conio.h>
#include <direct.h>

#define setenv(name, value, overwrite) SetEnvironmentVariable(name, value)
#define unsetenv(name) SetEnvironmentVariable(name, NULL)

#ifdef _MSC_VER
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#define alloca(x) _alloca(x)
#define access(x, y) _access(x, y)
#define realpath(abs, rel) _fullpath(abs, rel, PATH_MAX)
#else
#include <unistd.h>
#include <sys/time.h>
#endif

#define timersub(a, b, result)                                                \
  do {                                                                        \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;                             \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;                          \
    if ((result)->tv_usec < 0) {                                              \
      --(result)->tv_sec;                                                     \
      (result)->tv_usec += 1000000;                                           \
    }                                                                         \
  } while (0)

#else // UNIX-like systems backwards compat from Windows

#define Sleep(t) usleep(t * 1000)

#include <unistd.h>
#include <strings.h>
#include <alloca.h>

#endif


#ifdef _MSC_VER
	#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#else
	#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef __cplusplus
} // extern "C"
#endif

