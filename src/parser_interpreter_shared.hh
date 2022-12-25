#pragma once
#include <unordered_map>

[[gnu::noinline]] constexpr uint64_t hash(const char *text) {
	uint64_t h = 525201411107845655ull, i = 0;
	while (text[i++]) {
		h = (h ^ text[i]) * 0x5bd1e9955bd1e995;
		h ^= h >> 47;
	}
	return h;
}

// lets do little bit of trolling
// to avoid double hashing
struct Hasher {
	size_t operator()(const size_t &h) const {
		return h;
	}
};

struct Node;
struct CallParams {
	Node **params;
	int paramLen;
};
using CallPtr = int(*)(CallParams callParams);
extern volatile std::unordered_map<size_t, std::pair<uint16_t, CallPtr>, Hasher> multicharMapping;