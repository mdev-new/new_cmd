#pragma once

#ifndef _MSC_VER
#	define stdcall __attribute__((stdcall))
#else
#	define stdcall __stdcall
#endif

using RegisterCmdPtr = void(*)(char *cmd, CallPtr fn);
using SetEnvVarPtr = bool(*)(char *name, char *value);
using SleepPtr = void(*);
using CreateThreadPtr = bool(*)(void *threadStart);

struct DllMainData {
  RegisterCmdPtr registerCommand;
  SetEnvVarPtr setEnvVar;
  SleepPtr sleep;
  CreateThreadPtr createThread;
  void*(*getProcAddr)(char *modName, char *fnName);
};

using DllEntry = void(stdcall*)(DllMainData *data);
#define ExtEntryFunc void stdcall DllEntry(DllMainData *data)
