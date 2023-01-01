#pragma once
#include <vector>
#include "lexer.hpp"
#include "nodes.hh"
#include "shared.hh"

#define WITHCHILDREN (1 << 15)
#define BARETYPE (0xFF)

class Parser {
	Lexer lexer;

public:
	Parser(char *buffer, size_t length);
	void parse();

	std::vector<Node*> nodes;
};