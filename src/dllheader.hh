#pragma once
#define HEADER_VERSION 1

enum {
	UNCOMPRESSED,
	ALGO_LZ77,
	ALGO_DEFLATE
};

typedef struct {
	short magic;
	char sizeOfSelf;
	char headerVersion;
	int compressionFlags;
	long uncompressed_size;
	size_t entryOffset;
} __attribute__((__packed__)) Header;

#define MAGIC 'Mk' // in file its in reverse order
#define VERSION 1