#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "cmd_sdk.hh"

DllMainData *dllData = nullptr;

[[noreturn]] void threadProc() {

	// run background job (input polling, etc)
	while(1) {
		dllData.Sleep(1000);
	}
}

long counter = 0;
IFUN(testCommand) {
	return ++counter;
}

ExtEntryFunc {
	dllData = data;
	dllData.registerCommand("test", testCommand);

	dllData.createThread(ThreadProc);
}
