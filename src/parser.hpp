#pragma once
#include "lexer.hpp"
#include <cstring>
#include <vector>
#include <memory>
#include <typeinfo>
#include "extension_api.hh"

#define WITHCHILDREN (1 << 15)
#define BARETYPE (0xFF)

// TODO EXPRESSION TREE

// I hate that this has to be here but whatever

// implemented in nodeimpl.cxx
int evalExpr(Node *root);

class Parser {
	Lexer lexer;

public:
	Parser(char *buffer, size_t length);
	void parse();

	std::vector<Node*> nodes;
};
