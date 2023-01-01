#pragma once
#include <unordered_map>
#include <utility>
#include <stack>
#include "nodes.hh"
#include "parser.hpp"

struct InterpreterState {
	bool extensions;
	bool delayedExpansion;

	std::unordered_map<size_t, long> labelMap;
	std::stack<char*> directoryStack;
	std::stack<std::unordered_map<char *, char *>> locals; //todo every call level recieves fresh one
};

class Interpreter {
	Parser parser;
	InterpreterState state;

public:
	Interpreter(char *buffer, size_t size);
	int interpret();

	std::vector<Node*> &nodes = parser.nodes;
};
