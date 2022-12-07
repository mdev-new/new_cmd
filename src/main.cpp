// TODO:
// basic interpreter for files
// repl support

// far targets:
// caching
// multithreading
// maybe get rid of lexer?

//#define WIN32_LEAN_AND_MEAN
//#include <Windows.h>
#include <cstdio>
#include <cstdlib>

#include "lexer.hpp"
#include "parser.hpp"

int main(int argc, char *argv[]) {
	FILE *f = fopen(argv[1], "r");
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	rewind(f);

	char *buffer = malloc(size+1);
	fread(buffer, size, 1, f);

	LexedFile lexed = lex(buffer, size);
	//parse(lexed);

	for(int i = 0; i < lexed.sizeOfTokens; i++) {
	 	printf("%d %d %d\n", lexed.tokens[i].token, lexed.tokens[i].value, lexed.tokens[i].additionalData);
	}

	return 0;
}
