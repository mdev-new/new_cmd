#pragma once

#include "standard.h"
#include "interpreter/nodes.hh"

using RegisterCmdPtr = void(stdcall*)(char *cmd, CallPtr fn);
using SetEnvVarPtr = bool(stdcall*)(char *name, char *value);
using SleepPtr = void(stdcall*)(int len);
using CreateThreadPtr = bool(stdcall*)(void (*threadStart)(void*));

PACK(struct DllMainData {
  RegisterCmdPtr registerCommand;
  SleepPtr sleep;
  SetEnvVarPtr setEnvVar;
  CreateThreadPtr createThread;
  void*(*getProcAddr)(char *modName, char *fnName);
  size_t baseAddress;
});

using DllEntry = int(stdcall*)(DllMainData *data);