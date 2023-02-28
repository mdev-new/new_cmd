#pragma once

#ifndef _MSC_VER
#	define stdcall __attribute__((stdcall))
#else
#	define stdcall __stdcall
#endif

#include "interpreter/nodes.hh"

using RegisterCmdPtr = void(*)(char *cmd, void * fn);
using SetEnvVarPtr = bool(*)(char *name, char *value);
using SleepPtr = void(*)(int len);
using CreateThreadPtr = bool(*)(void *threadStart);

struct DllMainData {
  RegisterCmdPtr registerCommand;
  SetEnvVarPtr setEnvVar;
  SleepPtr sleep;
  CreateThreadPtr createThread;
  void*(*getProcAddr)(char *modName, char *fnName);
};

using DllEntry = void(stdcall*)(DllMainData *data);
#define ExtEntryPoint extern "C" int DllEntryPoint(DllMainData *data)
