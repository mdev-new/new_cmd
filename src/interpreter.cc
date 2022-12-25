#include "interpreter.hh"

// this thing MUST be optimized into oblivion
// if we're losing time in lexer or parser, doesnt really matter
/// (well, if we're losing time in the evaluate() methods it does matter)
// they mostly run only once, on startup
// this, however, will run in a loop.
// this project's future depends on the speed (and also extensibility)

#define IFUN(name) int name(CallParams callParams)
#define fe(x,y,z) {hash(x), std::make_pair(y,z)},

IFUN(doSet) {
	return 0;
}

IFUN(doExit) {
	return 0;
}

volatile std::unordered_map<size_t, std::pair<uint16_t, CallPtr>, Hasher> multicharMapping = {
	fe("for", TOK_FOR, nullptr)
	fe("do", TOK_DO, nullptr)
	fe("if", TOK_IF, nullptr)
	fe("else", TOK_ELSE, nullptr)
	fe("in", TOK_IN, nullptr)
	fe("set", TOK_BUILTIN, doSet)
	fe("exit", TOK_BUILTIN, doExit)
};

Interpreter::Interpreter(char *buffer, size_t size)
 : parser(buffer, size)
{
	parser.parse();
}

int Interpreter::interpret() {
	return 0;
}