#pragma once

#include <cstdint>
#include <cstddef>

#define _hashfunc_ fnvhash
#define _hashtype_ uint32_t
#define consthash constfnvhash

struct Hasher {
	size_t operator()(const size_t &h) const { return h; }
};

[[gnu::noinline]] uint32_t fnvhash(char const* str, bool removeCases = false);
consteval uint32_t constfnvhash(char const* str, bool removeCases = false) {
	uint32_t hash = 0, i = 0;
	char c = 0;
	while(c = str[i]) {
		if(removeCases && 'A' <= c && c <= 'Z') c += 'a' - 'A';
		hash = (hash * 0x811C9DC5) ^ c;
		i++;
	}
	return hash;
}