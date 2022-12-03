// TODO:
// basic interpreter for files
// repl support

// far targets:
// caching
// multithreading
// maybe get rid of lexer?

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdio>

#include "lexer.hpp"
#include "parser.hpp"

int main(int argc, char *argv[]) {
	LexedFile lexed = lex(CreateFileA(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
	parse(lexed);

	// for(int i = 0; i < lexed.sizeOfTokens; i++) {
	// 	printf("%d %d\n", lexed.tokens[i].token, lexed.tokens[i].value);
	// }

	return 0;
}