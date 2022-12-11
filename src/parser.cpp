#include "parser.hpp"
#include "lexer.hpp"
#include <cstdio>
#include <cstdlib>
#include <memory>

Node *MakeNode(Token *tok) {
	switch(tok->token) {
	case TOK_NUMBER: return new NumberNode(tok->value); break;
	}
}

// makes it easier to track internal state, and ability to use `this` instead of pointers
// this class is not exposed
class Parser {
private:
public:
	ProgramNode* parse(const LexedFile &lexedFile) {
		Token *current;
		Node **buffer = malloc(128*sizeof(Node*));
		int nodesMade = 0, skip = 0;

		#define emit(x) buffer[nodesMade++] = x; break

		for(int i = 0; i < lexedFile.noOfTokens; i++, i += skip, skip = 0) {
			current = &lexedFile.tokens[i];

			switch(current->token) {
			case TOK_PLUS: skip=1; emit(new PlusNode(MakeNode(&lexedFile.tokens[i-1]), MakeNode(&lexedFile.tokens[i+1]))); //todo get next tok
			case TOK_MINUS: skip=1; emit(new MinusNode(MakeNode(&lexedFile.tokens[i-1]), MakeNode(&lexedFile.tokens[i+1]))); //todo get next tok
			}
		}

		return new ProgramNode(buffer, nodesMade);
	}
};

ParsedFile parse(LexedFile &lexedFile) {
	Parser parser;
	return (ParsedFile){parser.parse(lexedFile)};
}
