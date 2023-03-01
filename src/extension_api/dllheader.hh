#pragma once
#define HEADER_VERSION 1

#include "standard.h"

enum {
	UNCOMPRESSED,
	ALGO_LZ77,
	ALGO_DEFLATE
};

PACK(typedef struct {
	short magic;
	char sizeOfSelf;
	char headerVersion;
	int compressionFlags;
	long uncompressed_size;
	size_t entryOffset;
} Header);

#define MAGIC 'Mk' // in file its in reverse order
#define VERSION 1
