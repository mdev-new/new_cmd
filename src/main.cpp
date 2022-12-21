// TODO:
// basic interpreter for files
// repl support
// fix memory leaks (free tokens, nodes, etc)
// sorting algo for number expressions (lexer)

// far targets:
// caching
// multithreading
// local caching of environment vars?
// JIT compilation?
// compile to memory?

// goals
// 1. speed
// 2. as few of malloc's and free's as possible

// thoughts:
/// implement ()s right in lexer level?

//#define WIN32_LEAN_AND_MEAN
//#include <Windows.h>
#include <cstdio>
#include <cstdlib>

#include "parser.hpp"

// void prettyPrint(Node *n) {
// 	if(n->type & 1) { // inner
// 		InnerNode *nn = n;
// 		if(nn->childrenCount) {
// 			for(Node *node : nn->children) {
// 				prettyPrint(node);
// 			}
// 		} else {
// 			prettyPrint(((InnerNode*)n)->lhs);
// 			prettyPrint(((InnerNode*)n)->rhs);
// 		}
// 	} else { // leaf
// 		printf();
// 	}
// }

int main(int argc, char *argv[]) {
	if(argc < 2) return 0;

	setenv("mbat_version", "001al", true);

	FILE *f = fopen(argv[1], "r");
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	rewind(f);

	char *buffer = malloc(size+1);
	fread(buffer, size, 1, f);

	Parser p(buffer, size);
	p.parse();

	//printf("--- Read ---\n%s\n", buffer);

	free(buffer);
	fclose(f);
	return 0;
}
