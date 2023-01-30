#pragma once
#include "nodes.hh"
#include "shared.hh"
#include "parser.hpp"

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
