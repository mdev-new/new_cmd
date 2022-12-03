#include "parser.hpp"
#include <cstdio>

AST parse(LexedFile &lexedFile) {
	int numtok = 0;

	for(int i = 0; i < lexedFile.sizeOfTokens; i++) {
		switch(lexedFile.tokens[i].token) {
		case TOK_NUMBER: numtok = numtok * 10 + (lexedFile.tokens[i].value - '0'); break;
		case TOK_SPACE: numtok = 0; break;
		}
	}

	printf("%d\n", numtok);
}