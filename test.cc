#include "src/extension_api/extension_api.hh"

DllMainData *dllData = nullptr;

IFUN(testCommand);
ExtEntryPoint {
	//int a = 0x1234;
	dllData = data;
	dllData->registerCommand("test", testCommand);
	//threadProc();

	//dllData->createThread(ThreadProc);
	return 0x1234;
}

// [[noreturn]] void threadProc() {
// 	// run background job (input polling, etc)
// 	while(1) {
// 		dllData->sleep(5000 * 1000);
// 	}
// }

long counter = 0;
IFUN(testCommand) {
	return ++counter;
}