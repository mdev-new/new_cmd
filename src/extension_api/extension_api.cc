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

void* stdcall getProcAddress(char *mod, char *fn) {
	// todo cache modules?
#if defined(_WIN32)
  return GetProcAddress(GetModuleHandleA(mod), fn);
#elif defined(__APPLE__)
# error getProcAddress on Apple NOT IMPLEMENTED
#elif defined(__linux__)
  return dlsym(NULL, fn);
#endif
}


void stdcall getProcAddresses(size_t *funcsBase, char *funcStrBase, char *modules, unsigned noOfFuncsAndModules) {
	// noOfFuncsAndModules is an ugly hack because i havent figured out how to handle 5+ args
	//printf("funcsBase:%llx funcStrBase:%llx modules:%llx noOfFuncs:%d noOfModules:%d\n", funcsBase, funcStrBase, modules, noOfFuncsAndModules & 0xFFFF, (noOfFuncsAndModules & 0xFFFF0000) >> 15);
	char *modules2 = nullptr;
	unsigned noMods = ((noOfFuncsAndModules & 0xFFFF0000) >> 15);
	unsigned noFuncs = (noOfFuncsAndModules & 0x0000FFFF);
	for (unsigned i = 0; i < noFuncs; i++) {
		//printf("Getting address of: %s (funcStrBase = %llx) ", funcStrBase, funcStrBase);

		modules2 = modules;
		for (unsigned j = 0; j < noMods; j++) {
			void *func = getProcAddress(modules2, funcStrBase);
			if(func != NULL) {
				funcsBase[i] = func;
				break;
			} else {
				modules2 += strlen(modules2)+1;
				continue;
			}
		}

		int x = strlen(funcStrBase)+1;
		funcStrBase += x;
		//printf("(result = %llx); adding %d to funcStrBase\n", funcsBase[i], x);
	}
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

void *freeExecutable(void *ptr) {
#ifdef __linux__
  //mprotect(ptr, size, PROT_READ | PROT_EXEC);
  free(ptr);
#elif defined(_WIN32)
  ptr = VirtualFree(ptr, 0, MEM_RELEASE);
#endif
}

void stdcall RegisterCommand(char *cmd, CallPtr func) {
	multicharMapping[_hashfunc_(cmd)] = std::make_pair(Token::Type::BuiltIn, func);
	return;
}

void stdcall createThread(void *func) {
#ifdef __linux__
	#error extension_api.cc:createThread not implemented
#elif defined(_WIN32)
	CreateThread(0,0,func,0,0,0);
#endif
}

extern char *stdcall itoa_(int n);

#define isdigit(x) (x >= '0' && x <= '9' ? 1 : 0)
long GETINPUT_SUB atol(const char *num) {
	long value = 0, neg = 0;
	if (num[0] == '-') { neg = 1; ++num; }
	while (*num && isdigit(*num)) value = value * 10 + *num++  - '0';
	return neg? -value : value;
}

long getenvnum(char *name) {
	static char buffer[32] = {0};
	return GetEnvironmentVariable(name, buffer, sizeof(buffer))? atol(buffer) : 0;
}

int ceil_(double num) {
	int a = num;
	return ((double)a != num) ? a+1 : a;
}

static FuncPtrs funcPtrs = {
	.registerCommand = RegisterCommand,
	.sleep = std_Sleep,
	.setEnvVar = std_setenv,
	.createThread = createThread,
	.getProcAddr = getProcAddress,
	.getProcAddrs = getProcAddresses,
	.itoa = itoa_
	.atoi = atoi
};

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

	int retcode = 0;

	DllEntry e = nullptr;
	switch(header->compressionFlags & 0xF) {
	case UNCOMPRESSED: {
		retcode = (e = (DllEntry)(buffer+header->sizeOfSelf))(&funcPtrs, e);
		break;
	}
	case ALGO_LZ77: {
		char *decompBuffer = (decltype(decompBuffer)) allocExecutable(header->uncompressed_size);
		if (decompBuffer == NULL) return ERROR_CANNOT_ALLOCATE;

		int decomp = lz77_decompress(buffer+header->sizeOfSelf, size-header->sizeOfSelf, decompBuffer, header->uncompressed_size+1, header->compressionFlags >> 27);
		if (decomp != header->uncompressed_size) return ERROR_CANNOT_DECOMPRESS;
		retcode = (e = (DllEntry)(decompBuffer))(&funcPtrs, e);
		//free(decompBuffer);
		break;
	}
	case ALGO_DEFLATE: {
		char *decompBuffer = (decltype(decompBuffer)) allocExecutable(header->uncompressed_size);
		if (decompBuffer == NULL) return ERROR_CANNOT_ALLOCATE;

		int decomp = sinfl_decompress(decompBuffer, header->uncompressed_size+1, buffer+header->sizeOfSelf, size-header->sizeOfSelf);
		if (decomp != header->uncompressed_size) return ERROR_CANNOT_DECOMPRESS;
		retcode = (e = (DllEntry)(decompBuffer))(&funcPtrs, e);
		//free(decompBuffer);
		break;
	}
	default: break;
	}

	if((header->compressionFlags & 0b1111) != UNCOMPRESSED) freeExecutable(buffer);
	return retcode;
}
