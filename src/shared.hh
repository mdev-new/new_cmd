#pragma once
#include <cstdint>
#include <unordered_map>
#include <unordered_map>
#include <stack>
#include "nodes.hh"

struct Hasher {
	size_t operator()(const size_t &h) const { return h; }
};

[[gnu::noinline]] constexpr uint64_t hash64(const char *text);
[[gnu::noinline]] constexpr uint32_t hash32(const char* data);

#define TCAST(type, val) ((type)val)
#define MKNTYP(T, ST) (T | (ST << 1))
#define IN(X) (NODE_INNER | (X << 1))
#define LN(X) (NODE_LEAF | (X << 1))

extern std::unordered_map<_hashtype_, std::pair<uint8_t, CallPtr>, Hasher> multicharMapping;