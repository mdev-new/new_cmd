#include "interpreter.hh"
#include "shared.hh"
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <climits>
#include <cstdlib>
#include "win.hh"
#include "nodes.hh"
#include "commands.hh"

// optimize as much as possible

#define IFUN(name) int name(CallParams callParams)

[[gnu::noinline]] uint32_t fnvhash(char const* str, bool lowercase = false) {
	uint32_t hash = 0;
	char c, i = 0; // lets not hash strings > 256 chars
	while(c = str[i++]) {
		hash = (hash * 0x811C9DC5) ^ ((lowercase)? c | 0x20 : c);
	}
	return hash;
}

Interpreter::Interpreter(char *buffer, size_t size)
 : parser(buffer, size)
{
	parser.parse();
	state.buffer = buffer;
	state.bufferSize = size;

	for(auto n : this->nodes) {
		if((n->type & BARETYPE) == Node::Type::Label) {
			LabelNode *ln = n;
			this->state.labels[_hashfunc_(ln->str)] = ln->pos;
		}
	}
}

int Interpreter::interpret() {
	int &i = this->state.filepos;
	for(; i < this->nodes.size(); i++) {
		nodes[i]->evaluate(&this->state);
	}

	return 0;
}
