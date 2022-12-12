#include "interpreter.hh"

std::unordered_map<uint64_t, std::pair<int, CallPtr>> multicharMapping = {
	{hash("for"), std::make_pair(TOK_FOR, nullptr)},
	{hash("do"), std::make_pair(TOK_DO, nullptr)},
	{hash("set"), std::make_pair(TOK_BUILTIN, nullptr)},
	{hash("if"), std::make_pair(TOK_IF, nullptr)},
	{hash("else"), std::make_pair(TOK_ELSE, nullptr)},
};

INTERNALFUN(doSet) {}
