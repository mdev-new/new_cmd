#include "shared.hh"
#ifdef _WIN64
#include "win.hh"

// todo expose node types
#include "parser.hpp"
#include "dllheader.hh"

#include "extern/lz77.hh"

#define SINFL_IMPLEMENTATION
#include "extern/sinfl.hh"

#include <cstdlib>
#include <cstdio>

typedef unsigned __int64 QWORD;
extern void RegisterCommand(char *cmd, CallPtr func);

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

typedef struct {
    PBYTE imageBase;
    HMODULE(WINAPI* loadLibraryA)(PCSTR);
    FARPROC(WINAPI* getProcAddress)(HMODULE, PCSTR);
    VOID(WINAPI* rtlZeroMemory)(PVOID, SIZE_T);
    LPARAM reservedParam;
} LoaderData;

using EntryPointPtr = DWORD(__stdcall*)(HMODULE, DWORD, LPVOID);

// not even gonna attempt to error check that
QWORD WINAPI loadLibrary(LoaderData* loaderData) {
    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(loaderData->imageBase + ((PIMAGE_DOS_HEADER)loaderData->imageBase)->e_lfanew);
    PIMAGE_BASE_RELOCATION relocation = (PIMAGE_BASE_RELOCATION)(loaderData->imageBase + ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
    QWORD delta = (QWORD)(loaderData->imageBase - ntHeaders->OptionalHeader.ImageBase);
    while (relocation->VirtualAddress) {
        PWORD relocationInfo = (PWORD)(relocation + 1);
        for (int i = 0, count = (relocation->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD); i < count; i++)
            if (relocationInfo[i] >> 12 == IMAGE_REL_BASED_HIGHLOW)
                *(QWORD *)(loaderData->imageBase + (relocation->VirtualAddress + (relocationInfo[i] & 0xFFF))) += delta;

        relocation = (PIMAGE_BASE_RELOCATION)((LPBYTE)relocation + relocation->SizeOfBlock);
    }

    PIMAGE_IMPORT_DESCRIPTOR importDirectory = (PIMAGE_IMPORT_DESCRIPTOR)(loaderData->imageBase + ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    while (importDirectory->Characteristics) {
        PIMAGE_THUNK_DATA originalFirstThunk = (PIMAGE_THUNK_DATA)(loaderData->imageBase + importDirectory->OriginalFirstThunk);
        PIMAGE_THUNK_DATA firstThunk = (PIMAGE_THUNK_DATA)(loaderData->imageBase + importDirectory->FirstThunk);

        HMODULE module = loaderData->loadLibraryA((LPCSTR)loaderData->imageBase + importDirectory->Name);

        if (!module)
            return FALSE;

        while (originalFirstThunk->u1.AddressOfData) {
            QWORD Function = (QWORD)loaderData->getProcAddress(module, originalFirstThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG ? (LPCSTR)(originalFirstThunk->u1.Ordinal & 0xFFFF) : ((PIMAGE_IMPORT_BY_NAME)((LPBYTE)loaderData->imageBase + originalFirstThunk->u1.AddressOfData))->Name);

            if (!Function)
                return FALSE;

            firstThunk->u1.Function = Function;
            originalFirstThunk++;
            firstThunk++;
        }
        importDirectory++;
    }

    if (ntHeaders->OptionalHeader.AddressOfEntryPoint) {
	      EntryPointPtr epnt = (EntryPointPtr)(loaderData->imageBase + ntHeaders->OptionalHeader.AddressOfEntryPoint);
        QWORD result = epnt((HMODULE)loaderData->imageBase, DLL_PROCESS_ATTACH, loaderData->reservedParam);

        //loaderData->rtlZeroMemory(loaderData->imageBase + ntHeaders->OptionalHeader.AddressOfEntryPoint, 32);
        loaderData->rtlZeroMemory(loaderData->imageBase, ntHeaders->OptionalHeader.SizeOfHeaders);
        return result;
    }
    return TRUE;
}

// todo error checking?
INT HookDll(HANDLE hProcess, LPVOID dllcode, LPARAM lParam) {
  PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(dllcode + ((PIMAGE_DOS_HEADER)dllcode)->e_lfanew);
  PBYTE executableImage = VirtualAlloc(NULL, ntHeaders->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
  if(executableImage == NULL) return ERROR_CANNOT_VIRTUAL_ALLOC;

  memcpy(executableImage, dllcode, ntHeaders->OptionalHeader.SizeOfHeaders);

  PIMAGE_SECTION_HEADER sectionHeaders = (PIMAGE_SECTION_HEADER)(ntHeaders + 1);
  for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++)
    memcpy(executableImage + sectionHeaders[i].VirtualAddress, dllcode + sectionHeaders[i].PointerToRawData, sectionHeaders[i].SizeOfRawData);

  LoaderData loaderParams = {
    .imageBase = executableImage,
    .loadLibraryA = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA"),
    .getProcAddress = GetProcAddress(GetModuleHandle("kernel32.dll"), "GetProcAddress"),
    .rtlZeroMemory = (VOID(NTAPI*)(PVOID, SIZE_T))GetProcAddress(GetModuleHandle("ntdll.dll"), "RtlZeroMemory"),
    .reservedParam = lParam
  };

  loadLibrary(&loaderParams);
}

IFUN(doInject) {
	Node **parameters = callParams.params;
	Node *param0 = parameters[0];
	if(param0->type != MKNTYP(NODE_LEAF, LNODE_STRING)) return -1;

	char *dllname = TCAST(StringNode*, param0)->str;
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());

	FILE *fp = fopen(dllname, "rb");
	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	rewind(fp);

	char *buffer = malloc(size);
	fread(buffer, size, 1, fp);

	Header *header = (Header *)buffer;
	if(header->magic != MAGIC) return -1; // TODO

	switch(header->compression & 0b1111) {
	case UNCOMPRESSED: HookDll(hProc, buffer+header->sizeOfSelf, (LONG_PTR)RegisterCommand); break;
	case ALGO_LZ77: {
		char *decompBuffer = malloc(header->uncompressed_file_size);
		// todo error check

		int decomp = lz77_decompress(buffer+header->sizeOfSelf, size-header->sizeOfSelf, decompBuffer, header->uncompressed_file_size+1, header->compression >> 27);
		// todo error check
		HookDll(hProc, decompBuffer, (LONG_PTR)RegisterCommand); 
		free(decompBuffer);
		break;
	} break;
	case ALGO_DEFLATE: {
		char *decompBuffer = malloc(header->uncompressed_file_size);
		// todo error check

		int decomp = sinfl_decompress(decompBuffer, header->uncompressed_file_size+1, buffer+header->sizeOfSelf, size-header->sizeOfSelf);
		// todo error check

		// todo error check
		HookDll(hProc, decompBuffer, (LONG_PTR)RegisterCommand);
    printf("extension sucessful!\n");
		free(decompBuffer);
		break;
	} break;
	default: break;
	}

	free(buffer);
	return 0;

}

#endif
