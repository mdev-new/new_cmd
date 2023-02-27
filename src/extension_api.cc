#include "shared.hh"
#include "nodes.hh"
#include "commands.hh"

#if defined(_WIN32)
  #include "win.hh"
#elif defined(__APPLE__)
# error extension api NOT IMPLEMENTED
#elif defined(__linux__)
  #include <dlfcn.h>
  #include <sys/mman.h>
#endif

// todo expose node types
#include "nodes.hh"
#include "parser.hpp"
#include "extension_api.hh"
#include "dllheader.hh"

#include "extern/lz77.hh"

#define SINFL_IMPLEMENTATION
#include "extern/sinfl.hh"

#include <cstdlib>
#include <cstdio>

void *getProcAddress(char *mod, char *fn) {
  // todo cache results
#if   defined(_WIN32)
  return GetProcAddress(GetModuleHandleA(mod), fn);
#elif defined(__APPLE__)
# error __PRETTY_FUNCTION__ NOT IMPLEMENTED
#elif defined(__linux__)
  return dlsym(dlopen(mod, RTLD_NOW | RTLD_GLOBAL), fn);
#endif
}

void hookDll(void *buf, size_t size, DllEntry entry) {

#ifdef __linux__
  mprotect(buf, size, PROT_READ | PROT_WRITE | PROT_EXEC);
#elif defined(_WIN32)
  VirtualProtect(buf, size, PAGE_EXECUTE_READWRITE, nullptr);
#endif

  DllMainData d = {
    .registerCommand = RegisterCommand,
    .getProcAddr = getProcAddress
  };

  entry(&d);

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

	buffer = malloc(size);
	fread(buffer, size, 1, fp);

	Header *header = (Header *)buffer;
	if(header->magic != MAGIC) return -1;

	//printf("%llx\n", header->entryOffset);
	//exit(-1);

	switch(header->compressionFlags & 0b1111) {
	case UNCOMPRESSED: hookDll(buffer+header->sizeOfSelf, header->uncompressed_size, (DllEntry)buffer+header->sizeOfSelf+header->entryOffset); break;
	case ALGO_LZ77: {
		char *decompBuffer = malloc(header->uncompressed_size);
		// todo error check

		int decomp = lz77_decompress(buffer+header->sizeOfSelf, size-header->sizeOfSelf, decompBuffer, header->uncompressed_size+1, header->compressionFlags >> 27);
		// todo error check
		hookDll(decompBuffer, header->uncompressed_size, (DllEntry)decompBuffer+header->entryOffset);
		//free(decompBuffer);
		break;
	} break;
	case ALGO_DEFLATE: {
		char *decompBuffer = malloc(header->uncompressed_size);
		// todo error check

		int decomp = sinfl_decompress(decompBuffer, header->uncompressed_size+1, buffer+header->sizeOfSelf, size-header->sizeOfSelf);
		// todo error check

		// todo error check
		hookDll(decompBuffer, header->uncompressed_size, (DllEntry)decompBuffer+header->entryOffset);
		//free(decompBuffer);
		break;
	} break;
	default: break;
	}

	free(buffer);
	return 0;

}
