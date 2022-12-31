#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "extension_api.hh"

RegisterCmdPtr registerCommand = nullptr;

DWORD WINAPI ThreadProc(LPVOID lpParam) {

	// run background job (input polling, etc)
	while(1) {
		Sleep(1000);
	}

	// unreachable code
	return 0;
}

long counter = 0;
IFUN(testCommand) {
	return ++counter;
}

int APIENTRY DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved) {
	if (dwReason == DLL_PROCESS_ATTACH) {

		registerCommand = lpReserved;
		registerCommand("test", testCommand);

		CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadProc,NULL,0,NULL);

	}
	return TRUE;
}
