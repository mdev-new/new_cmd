#include "interpreter.hh"
#include "standard.h"
#include "nodes.hh"
#include "commands.hh"

// optimize as much as possible

#define IFUN(name) int name(CallParams callParams)

Interpreter::Interpreter(char *buffer, size_t size, InterpreterState *s)
 : parser(buffer, size)
{
	if(s == nullptr) {
		this->state = new InterpreterState {
			.echo = true,
			.filepos = 0,
			.buffer = buffer,
			.bufferSize = size
		};
	} else {
		this->state = s;
	}

	parser.parse();

	for(auto n : this->nodes) {
		if((n->type & BARETYPE) == Node::Type::Label) {
			LabelNode *ln = n;
			this->state->labels[_hashfunc_(ln->str)] = ln->pos;
		}
	}
}

int Interpreter::interpret() {
	int &i = this->state->filepos;
	for(; i < this->nodes.size(); i++) {
		nodes[i]->evaluate(this->state);
	}

	return 0;
}
