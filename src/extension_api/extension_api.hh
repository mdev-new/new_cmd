#pragma once

#ifndef _MSC_VER
#	define stdcall __attribute__((stdcall))
#else
#	define stdcall __stdcall
#endif

#include "interpreter/nodes.hh"

using RegisterCmdPtr = void(stdcall*)(char *cmd, void * fn);
using SetEnvVarPtr = bool(stdcall*)(char *name, char *value);
using SleepPtr = void(stdcall*)(int len);
using CreateThreadPtr = bool(stdcall*)(void (*threadStart)(void*));

struct DllMainData {
  SleepPtr sleep;
  RegisterCmdPtr registerCommand;
  SetEnvVarPtr setEnvVar;
  CreateThreadPtr createThread;
  void*(*getProcAddr)(char *modName, char *fnName);
};

using DllEntry = int(stdcall*)(DllMainData *data);