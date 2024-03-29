#pragma once
#include "nodes.hh"
#include "parser/parser.hpp"

class Interpreter {
	Parser parser;
	InterpreterState *state;

public:
	Interpreter(char *buffer, size_t size, InterpreterState *s = nullptr);
	int interpret();

	std::vector<Node*> &nodes = parser.nodes;
};
