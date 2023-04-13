#pragma once

#include "standard.h"
#include "interpreter/nodes.hh"

using RegisterCmdPtr = void(stdcall*)(char *cmd, CallPtr fn);
using SetEnvVarPtr = bool(stdcall*)(char *name, char *value);
using SleepPtr = void(stdcall*)(int len);
using CreateThreadPtr = bool(stdcall*)(void (*threadStart)(void*));

PACK(struct FuncPtrs {
  RegisterCmdPtr registerCommand;
  SleepPtr sleep;
  SetEnvVarPtr setEnvVar;
  CreateThreadPtr createThread;
  void*(stdcall * getProcAddr)(char *modName, char *fnName);
  void(stdcall * getProcAddrs)(size_t *funcsBase, char *funcStrBase, char *modules, int noOfFuncs, int noOfModules);
  char (stdcall *itoa)(int num);
});

using DllEntry = int(stdcall*)(FuncPtrs *data, size_t base);