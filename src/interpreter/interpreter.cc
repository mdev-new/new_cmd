#include "interpreter.hh"
#include "standard.h"
#include "nodes.hh"
#include "commands.hh"

// optimize as much as possible

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
	for(Node *n : this->nodes) {
		n->evaluate(this->state);
	}

	return 0;
}
