#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef int (*CompressFun)(uint8_t *input, uint8_t *output, uint32_t input_length);

#include "../dllheader.hh"

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

// todo error check?
// ./compress in out compression_type
int main(int argc, char *argv[]) {
	uint32_t type = 0, bits = 0;
	CompressFun compressPtr = NULL;

	switch(atol(argv[3])) {
	case 1: {
		type = ALGO_LZ77;
		bits = LZ77_MATCH_LENGTH_BITS;
		compressPtr = lz77comp;
	} break;

	case 2: {
		uint32_t type = ALGO_DEFLATE;
		uint32_t bits = 0;
		CompressFun compressPtr = deflcomp;
	} break;

	default:
	case 0: {
		type = UNCOMPRESSED;
		bits = 0;
		compressPtr = nocompress;
	} break;

	}

	FILE *fptr_r = fopen(argv[1], "rb");
	FILE *fptr_w = fopen(argv[2], "wb");

	fseek(fptr_r, 0, SEEK_END);
	int inlen = ftell(fptr_r);
	fseek(fptr_r, 0, SEEK_SET);

	char *inBuffer = malloc(inlen);
	char *outBuffer = malloc(inlen*2 + sizeof(Header));
	fread(inBuffer, inlen, 1, fptr_r);

	Header header = {
		.magic = MAGIC,
		.sizeOfSelf = sizeof(Header),
		.headerVersion = VERSION,
		.compression = (bits << 27) | type,
		.uncompressed_file_size = inlen
	};

	//printf("%d\n", header.compression);

	// write the footer
	memcpy(outBuffer, &header, sizeof(Header));

	int res = compressPtr(inBuffer, outBuffer+sizeof(Header), inlen);
	fwrite(outBuffer,res+sizeof(Header), 1, fptr_w);

	free(inBuffer);
	free(outBuffer);
	fclose(fptr_r);
	fclose(fptr_w);

	return res;
}