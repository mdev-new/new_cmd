#include "parser.hpp"
#include "lexer.hpp"
#include <cstdio>
#include <cstdlib>
#include <memory>

// we need to probably parse from inside out somehow
// that means first make all leaf nodes for the inner node x and then
// construct x

// todo write a function that constructs leaf nodes for inner node x

void sortToks(Token *start, Token *end) {
	int curtok = -1, lastok = -1;
	Token *current;

	for(int i = 0; i < (end - start); i++) {
		current = &start[i];

		if(current->token >= TOK_PLUS && current->token <= TOK_DIVIDE) {
			
		}
	}
}

class Parser {
private:
public:
	Node** parse(const LexedFile &lexedFile) {

	sortToks(&lexedFile.tokens[0], &lexedFile.tokens[lexedFile.noOfTokens-1]);

	//Token *current;
	// for(int i = 0; i < lexedFile.noOfTokens; i++) {
	// 	current = &lexedFile.tokens[i];
	// }
	}
};

ParsedFile parse(LexedFile &lexedFile) {
	Parser parser;
	return (ParsedFile){parser.parse(lexedFile)};
}