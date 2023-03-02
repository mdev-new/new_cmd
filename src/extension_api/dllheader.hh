#pragma once
#define HEADER_VERSION 1

#include "standard.h"

enum {
	UNCOMPRESSED,
	ALGO_LZ77,
	ALGO_DEFLATE
};

PACK(struct _Header {
	short magic;
	char sizeOfSelf;
	char headerVersion;
	int compressionFlags;
	long uncompressed_size;
});

typedef struct _Header Header;

#define MAGIC 'Mk' // in file its in reverse order
#define VERSION 1
