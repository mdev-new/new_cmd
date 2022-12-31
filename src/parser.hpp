#pragma once
#include "lexer.hpp"
#include <vector>
#include "extension_api.hh"

#define WITHCHILDREN (1 << 15)
#define BARETYPE (0xFF)

extern std::unordered_map<size_t, std::pair<uint16_t, CallPtr>, Hasher> multicharMapping;
extern int evalExpr(Node *root);

class Parser {
	Lexer lexer;

public:
	Parser(char *buffer, size_t length);
	void parse();

	std::vector<Node*> nodes;
};
