#pragma once
#include "nodes.hh"
#include "shared.hh"
#include "parser.hpp"

// evaluate root node or node without specific parameters
//uint64_t eval(Node *n, InterpreterState *state = nullptr);

class Interpreter {
	Parser parser;
	InterpreterState state = {
		.echo = true,
		.filepos = 0
	};

public:
	Interpreter(char *buffer, size_t size);
	int interpret();

	std::vector<Node*> &nodes = parser.nodes;
};
