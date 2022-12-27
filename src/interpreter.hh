#pragma once
#include <unordered_map>
#include <utility>
#include "lexer.hpp"
#include "extension_api.hh"
#include "parser.hpp"

class Interpreter {
	Parser parser;

public:
	Interpreter(char *buffer, size_t size);
	int interpret();

	std::vector<Node*> &nodes = parser.nodes;
};
