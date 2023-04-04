#include "interpreter/commands.hh"

#include "standard.h"

#if defined(_WIN32)
#elif defined(__APPLE__)
# error APPLE extension api NOT IMPLEMENTED
#elif defined(__linux__)
  #include <dlfcn.h>
  #include <sys/mman.h>
#endif

// todo expose node types
#include "interpreter/nodes.hh"
#include "dllheader.hh"
#include "extension_api.hh"

#include "extern/lz77.hh"

#define SINFL_IMPLEMENTATION
#include "extern/sinfl.hh"

void *getProcAddress(char *mod, char *fn) {
  // todo cache results
#if defined(_WIN32)
  return GetProcAddress(GetModuleHandleA(mod), fn);
#elif defined(__APPLE__)
# error __PRETTY_FUNCTION__ NOT IMPLEMENTED
#elif defined(__linux__)
  return dlsym(dlopen(mod, RTLD_NOW | RTLD_GLOBAL), fn);
#endif
}

void *allocExecutable(size_t size) {
	void *ptr = nullptr;
#ifdef __linux__
  ptr = malloc(size);
  mprotect(ptr, size, PROT_READ | PROT_WRITE | PROT_EXEC);
#elif defined(_WIN32)
  ptr = VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
#endif
	if (ptr == nullptr) printf("alloc failed!\n");
	return ptr;
}

void *freeExecutable(void *ptr, size_t size = 0) {
#ifdef __linux__
  free(ptr);
#elif defined(_WIN32)
  ptr = VirtualFree(ptr, size, MEM_RELEASE);
#endif
}

void stdcall RegisterCommand(char *cmd, CallPtr func) {
	multicharMapping[_hashfunc_(cmd)] = std::make_pair(Token::Type::BuiltIn, func);
}

void stdcall print(int a) {
	printf("Hooked native code prints: %d\n", a);
	return;
}

// todo move this to standard.c
static void stdcall sleep(int ms) {
	Sleep(ms); // standard.h for linux has macro Sleep so we're clear here
}

void hookDll(DllEntry entry) {

  DllMainData d = {
	  .registerCommand = RegisterCommand,
    .sleep = sleep,
    .setEnvVar = nullptr,
    .createThread = nullptr,
    .getProcAddr = getProcAddress
  };

  int x = entry(&d);
  printf("Native code returned: %d\n", x);

}

IFUN(doInject) {
	StringNode *param0 = (*callParams.params)[0];
	if(param0->type != Node::Type::String) return -1;

	char *dllname = param0->stringify().second;
    char *buffer = nullptr;

	FILE *fp = fopen(dllname, "rb");
	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	rewind(fp);

	buffer = (decltype(buffer)) allocExecutable(size); // files can be uncompressed
	fread(buffer, size, 1, fp);

	Header *header = (Header *)buffer;
	if(header->magic != MAGIC) return -1;

	//printf("%llx\n", header->entryOffset);
	//exit(-1);

	switch(header->compressionFlags & 0b1111) {
	case UNCOMPRESSED: hookDll((DllEntry)(buffer+header->sizeOfSelf)); break;
	case ALGO_LZ77: {
		char *decompBuffer = (decltype(decompBuffer)) allocExecutable(header->uncompressed_size);
		// todo error check

		int decomp = lz77_decompress(buffer+header->sizeOfSelf, size-header->sizeOfSelf, decompBuffer, header->uncompressed_size+1, header->compressionFlags >> 27);
		// todo error check
		hookDll((DllEntry)(decompBuffer));
		//free(decompBuffer);
		break;
	} break;
	case ALGO_DEFLATE: {
		char *decompBuffer = (decltype(decompBuffer)) allocExecutable(header->uncompressed_size);
		// todo error check

		int decomp = sinfl_decompress(decompBuffer, header->uncompressed_size+1, buffer+header->sizeOfSelf, size-header->sizeOfSelf);
		// todo error check

		// todo error check
		hookDll((DllEntry)(decompBuffer));
		//free(decompBuffer);
		break;
	} break;
	default: break;
	}

	//free(buffer);
	return 0;

}
