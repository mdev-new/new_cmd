#pragma once
#include <unordered_map>
#include <utility>
#include "lexer.hpp"
#include "parser.hpp"

// maybe todo upgrade gcc & use consteval
// leave this here since lexer.cpp depends on it
constexpr uint64_t hash(const char *text) {
  uint64_t h = 525201411107845655ull;
  for (int i = 0;text[i];++i) {
    h ^= text[i];
    h *= 0x5bd1e9955bd1e995;
    h ^= h >> 47;
  }
  return h;
}

struct CallParams {
	Node *params;
};
using CallPtr = int(*)(CallParams &callParams);

#define INTERNALFUN(name) int name(CallParams &callParams)

INTERNALFUN(doSet);

extern std::unordered_map<uint64_t, std::pair<int, CallPtr>> multicharMapping;
