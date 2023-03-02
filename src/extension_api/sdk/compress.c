#include "standard.h"

typedef int (*CompressFun)(uint8_t *input, uint8_t *output, uint32_t input_length);

#include "utilities/file_utils.h"

#include "extension_api/dllheader.hh"

// range 2-8 inclusive
#define LZ77_MATCH_LENGTH_BITS 8
#include "lz77comp.h"

#define SDEFL_IMPLEMENTATION
#include "sdefl.h"

int nocompress(uint8_t *input, uint8_t *output, uint32_t input_length) {
	memcpy(output, input, input_length);
	return input_length;
}

int lz77comp(uint8_t *input, uint8_t *output, uint32_t input_length) {
	return lz77_compress(input, input_length, output, input_length*2);
}

static struct sdefl sdefl;
int deflcomp(uint8_t *input, uint8_t *output, uint32_t input_length) {
	return sdeflate(&sdefl, output, input, input_length, SDEFL_LVL_MAX);
}

// TODO error check?
// TODO convert to binary here and dont rely on objcopy
// ./compress in out compression_type
int main(int argc, char *argv[]) {
	uint32_t type = 0, bits = 0;
	CompressFun compressPtr = NULL;

	//FILE *fptr_r_exe = fopen(argv[1], "rb");
	FILE *fptr_r_bin = fopen(argv[1], "rb");
	FILE *fptr_w = fopen(argv[2], "wb");

	switch(atol(argv[3])) {
	case 1: {
		type = ALGO_LZ77;
		bits = LZ77_MATCH_LENGTH_BITS;
		compressPtr = lz77comp;
	} break;

	case 2: {
		type = ALGO_DEFLATE;
		bits = 0;
		compressPtr = deflcomp;
	} break;

	default:
	case 0: {
		type = UNCOMPRESSED;
		bits = 0;
		compressPtr = nocompress;
	} break;
	}

	size_t entryOffset = 0;

	/*{
		fseek(fptr_r_exe, 0, SEEK_END);
		size_t inlen = ftell(fptr_r_exe);
		fseek(fptr_r_exe, 0, SEEK_SET);

		char *inBuffer = alloca(inlen);
		fread(inBuffer, inlen, 1, fptr_r_exe);
		fclose(fptr_r_exe);

		uint32_t *longBuf = inBuffer;
		uint16_t *shortBuf = inBuffer;
		size_t *llBuf = inBuffer; // lets ignore 32bit for now
#ifdef __linux__
		if(longBuf[0] == 1179403647) { // its an elf file!
			size_t *elfStart = inBuffer+0x18;
			entryOffset = *elfStart;
			//printf("%llx\n", entryOffset);
		}
#elif defined(_WIN32)
		if(shortBuf[0] == 'MZ') { // its an pe file
			// god i hate Windows, just look at the ELF read code and compare it to this
			PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(inBuffer + ((PIMAGE_DOS_HEADER)inBuffer)->e_lfanew);

			if (ntHeaders->OptionalHeader.AddressOfEntryPoint) {
				entryOffset = (inBuffer + ntHeaders->OptionalHeader.AddressOfEntryPoint);
			}
		}
#endif
	}*/

	size_t sz = get_file_size(fptr_r_bin);
	char *inBuffer = malloc(sz);
	fread(inBuffer, sz, 1, fptr_r_bin);
	fclose(fptr_r_bin);

	Header header = {
		.magic = MAGIC,
		.sizeOfSelf = sizeof(Header),
		.headerVersion = VERSION,
		.compressionFlags = (bits << 27) | type,
		.uncompressed_size = sz,
		//.entryOffset = entryOffset
	};

	//printf("%d\n", header.compression);

	// write the footer
	char *outBuffer = malloc(sz*2 + sizeof(Header));
	memcpy(outBuffer, &header, sizeof(Header));

	int res = compressPtr(inBuffer, outBuffer+sizeof(Header), sz);
	fwrite(outBuffer,res+sizeof(Header), 1, fptr_w);
	fclose(fptr_w);

	free(inBuffer);
	free(outBuffer);

	return res;
}
