#pragma once
#include <cstdint>
#include <unordered_map>
#include "nodes.hh"

struct Hasher {
	size_t operator()(const size_t &h) const { return h; }
};

[[gnu::noinline]] uint32_t fnvhash(char const* str, bool removeCases = false);
consteval uint32_t consthash(char const* str, bool removeCases = false) {
	uint32_t hash = 0, i = 0;
	char c = 0;
	while(c = str[i]) {
		if(removeCases && 'A' <= c && c <= 'Z') c += 'a' - 'A';
		hash = (hash * 0x811C9DC5) ^ c;
		i++;
	}
	return hash;
}

//todo move
#define _hashfunc_ fnvhash
#define _hashtype_ uint32_t

#define TCAST(type, val) ((type)val)
#define MKNTYP(T, ST) (T | (ST << 1))
#define IN(X) (NODE_INNER | (X << 1))
#define LN(X) (NODE_LEAF | (X << 1))

extern std::unordered_map<_hashtype_, std::pair<uint8_t, CallPtr>, Hasher> multicharMapping;
