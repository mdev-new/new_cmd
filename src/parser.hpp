#pragma once
#include <vector>
#include <cstdint>
#include "lexer.hpp"
#include "nodes.hh"

#define WITHCHILDREN (1 << 15)
#define BARETYPE (0xFF)

class Parser {
	Lexer lexer;
	std::vector<Token> tokens;

public:
	Parser(char *buffer, size_t length);
	void parse();

	std::vector<Node*> nodes;
};