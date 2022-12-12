#include "parser.hpp"
#include "lexer.hpp"
#include <cstdio>
#include <cstdlib>
#include <memory>

class Parser {
private:
	Node *parseExpr() {
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