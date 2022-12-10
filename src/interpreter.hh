#pragma once
#include <unordered_map>
#include "lexer.hpp"

// todo upgrade gcc & use consteval
constexpr uint64_t hash(const char *text) {
  uint64_t h = 525201411107845655ull;
  for (int i = 0;text[i];++i) {
    h ^= text[i];
    h *= 0x5bd1e9955bd1e995;
    h ^= h >> 47;
  }
  return h;
}

struct CallParams {};
using CallPtr = int(*)(CallParams &callParams);

std::unordered_map<uint64_t, std::pair<int, CallPtr>> multicharMapping = {
	{hash("for"), std::make_pair(TOK_FOR, nullptr)},
	{hash("do"), std::make_pair(TOK_DO, nullptr)},
	{hash("set"), std::make_pair(TOK_SET, nullptr)},
	{hash("if"), std::make_pair(TOK_IF, nullptr)},
	{hash("else"), std::make_pair(TOK_ELSE, nullptr)},
};
