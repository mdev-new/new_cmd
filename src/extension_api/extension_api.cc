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

enum {
  ERROR_NOT_CMDEXE = -2,
  ERROR_CANNOT_GET_PARENT_PROC = -3,
  ERROR_FILE_DOESNT_EXIST_OR_INVALID_ARGS = -4,
  ERROR_CANNOT_OPEN_FILE = -5,
  ERROR_EMPTY_FILE = -6,
  ERROR_CANNOT_READ_FILE = -7,
  ERROR_CANNOT_ALLOCATE = -8,
  ERROR_CANNOT_FREE = -9,
  ERROR_CANNOT_DECOMPRESS = -10,
  ERROR_CANNOT_HOOK = -11,
  ERROR_CANNOT_WRITE_PROCESS_MEM = -12,
  ERROR_CANNOT_VIRTUAL_ALLOC = -13,
  ERROR_CANNOT_VIRTUAL_FREE = -14,
  ERROR_CANNOT_OPEN_PROCESS = -15,
  ERROR_CANNOT_OPEN_HEAP = -15,
  ERROR_CANNOT_GET_PARENT_PROC_PATH = -16,
  ERROR_INVALID_FILE = -17
};

void *getProcAddress(char *mod, char *fn) {
  // todo cache modules?
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
  mprotect(ptr, size, PROT_READ | PROT_WRITE | PROT_EXEC); // todo: is this really needed?
#elif defined(_WIN32)
  ptr = VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
#endif
	//if (ptr == nullptr) printf("alloc failed!\n");
	return ptr;
}

void *freeExecutable(void *ptr, size_t size = 0) {
#ifdef __linux__
  //mprotect(ptr, size, PROT_READ | PROT_EXEC);
  free(ptr);
#elif defined(_WIN32)
  ptr = VirtualFree(ptr, size, MEM_RELEASE);
#endif
}

void stdcall RegisterCommand(char *cmd, CallPtr func) {
	multicharMapping[_hashfunc_(cmd)] = std::make_pair(Token::Type::BuiltIn, func);
	return;
}

int hookDll(DllEntry entry) {
  DllMainData d = {
    .registerCommand = RegisterCommand,
    .sleep = std_Sleep,
    .setEnvVar = std_setenv,
    .createThread = nullptr,
    .getProcAddr = getProcAddress,
	  .baseAddress = entry
  };

  return entry(&d);
}

IFUN(doInject) {
	StringNode *param0 = (*callParams.params)[0];
	if(param0->type != Node::Type::String && param0->type != Node::Type::Id) return -1; //todo make smth like isTextNode func

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

  int retcode = 0;

	switch(header->compressionFlags & 0b1111) {
	case UNCOMPRESSED: retcode = hookDll((DllEntry)(buffer+header->sizeOfSelf)); break;
	case ALGO_LZ77: {
		char *decompBuffer = (decltype(decompBuffer)) allocExecutable(header->uncompressed_size);
		if (decompBuffer == NULL) return ERROR_CANNOT_ALLOCATE;

		int decomp = lz77_decompress(buffer+header->sizeOfSelf, size-header->sizeOfSelf, decompBuffer, header->uncompressed_size+1, header->compressionFlags >> 27);
    if (decomp != header->uncompressed_size) return ERROR_CANNOT_DECOMPRESS;
		retcode = hookDll((DllEntry)(decompBuffer));
		//free(decompBuffer);
		break;
	} break;
	case ALGO_DEFLATE: {
		char *decompBuffer = (decltype(decompBuffer)) allocExecutable(header->uncompressed_size);
		if (decompBuffer == NULL) return ERROR_CANNOT_ALLOCATE;

		int decomp = sinfl_decompress(decompBuffer, header->uncompressed_size+1, buffer+header->sizeOfSelf, size-header->sizeOfSelf);
    if (decomp != header->uncompressed_size) return ERROR_CANNOT_DECOMPRESS;
		retcode = hookDll((DllEntry)(decompBuffer));
		//free(decompBuffer);
		break;
	} break;
	default: break;
	}

	//free(buffer);
	return retcode;
}
