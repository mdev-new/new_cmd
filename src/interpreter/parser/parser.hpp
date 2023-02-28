#pragma once
#include <vector>
#include <cstdint>
#include <variant>
#include "interpreter/lexer/lexer.hpp"
#include "interpreter/nodes.hh"

class Parser {
	Lexer lexer;
	std::vector<Token> tokens;

	size_t idx;
	Node *consume(short tokentype);

public:
	Parser(char *buffer, size_t length);
	void parse();

	std::vector<Node*> nodes;
};