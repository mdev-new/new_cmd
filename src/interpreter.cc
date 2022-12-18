#include "interpreter.hh"

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
