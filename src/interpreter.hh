#pragma once
#include "nodes.hh"
#include "shared.hh"
#include "parser.hpp"

void RegisterLabel(InterpreterState *state, LabelNode *n);

class Interpreter {
	Parser parser;
	InterpreterState state = {
		.extensions = true,
		.delayedExpansion = false,
		.echo = true,
		.filepos = 0
	};

public:
	Interpreter(char *buffer, size_t size);
	int interpret();

	std::vector<Node*> &nodes = parser.nodes;
};
