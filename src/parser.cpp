#include "parser.hpp"
#include "lexer.hpp"
#include <cstdio>
#include <cstdlib>
#include <memory>

// we need to probably parse from inside out somehow
// that means first make all leaf nodes for the inner node x and then
// construct x

// todo write a function that constructs leaf nodes for inner node x

class Parser {
private:
	Node *parseExpr(Token *tokens, int current) {
		switch(tokens[current].token) {
		//case TOK_PLUS: return new AdditionNode(
		}
	}

public:
	Node** parse(const LexedFile &lexedFile) {

	Token *current;

	for(int i = 0; i < lexedFile.noOfTokens; i++) {
		current = &lexedFile.tokens[i];
	}
	}
};

ParsedFile parse(LexedFile &lexedFile) {
	Parser parser;
	return (ParsedFile){parser.parse(lexedFile)};
}