#pragma once
#include <unordered_map>
#include <utility>
#include "lexer.hpp"
#include "parser.hpp"

[[gnu::noinline]] constexpr uint64_t hash(const char *text) {
	uint64_t h = 525201411107845655ull, i = 0;
	while (text[i++]) {
		h = (h ^ text[i]) * 0x5bd1e9955bd1e995;
		h ^= h >> 47;
	}
	return h;
}

struct CallParams {
	Node *params;
};
using CallPtr = int(*)(CallParams &callParams);
extern std::unordered_map<uint64_t, std::pair<uint16_t, CallPtr>> multicharMapping;

int Interpret(const ParsedFile &parsed);
