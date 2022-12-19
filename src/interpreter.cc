#include "interpreter.hh"

// this thing MUST be optimized into oblivion
// if we're losing time in lexer or parser, doesnt really matter
// they run only once, on startup
// this, however, will run in a loop.
// this project's future depends on the speed

#define IFUN(name) int name(CallParams &callParams)
#define fe(x,y,z) {hash(x), std::make_pair(y,z)},

IFUN(doSet) {}
IFUN(doExit) {}

std::unordered_map<uint64_t, std::pair<uint16_t, CallPtr>> multicharMapping = {
	fe("for", TOK_FOR, nullptr)
	fe("do", TOK_DO, nullptr)
	fe("if", TOK_IF, nullptr)
	fe("else", TOK_ELSE, nullptr)
	fe("set", TOK_BUILTIN, doSet)
	fe("exit", TOK_BUILTIN, doExit)
};

int Interpret(const ParsedFile &parsed) {

}
