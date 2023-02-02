#pragma once
#include <vector>
#include <cstdint>
#include <variant>
#include "lexer.hpp"
#include "nodes.hh"
#include "shared.hh"

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