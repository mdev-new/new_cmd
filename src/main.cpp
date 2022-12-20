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

#include "lexer.hpp"
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

	//printf("--- Read ---\n%s\n", buffer);

	LexedFile lexed = lex(buffer, size);
	//ParsedFile parsed = parse(lexed);

	printf("--- Lexed --- \n");
	for(int i = 0; i < lexed.noOfTokens; i++) {
		if(lexed.tokens[i].token == TOK_WS_SEPARATOR) continue;

		if((lexed.tokens[i].token == TOK_UNDEFINED || lexed.tokens[i].token == TOK_SWITCH || lexed.tokens[i].token == TOK_BUILTIN || lexed.tokens[i].token == TOK_STRING) && lexed.tokens[i].value != 0) printf("%d 0x%llx(%s) %d\n", lexed.tokens[i].token, lexed.tokens[i].value, lexed.tokens[i].value, lexed.tokens[i].additionalData);
		else printf("%d %d %d\n", lexed.tokens[i].token, lexed.tokens[i].value, lexed.tokens[i].additionalData);
	}

	printf("--- Parsed --- \n");
	//printf("%d\n", ((NumberNode*)((BinOpNode*)parsed.nodes[0])->evaluate())->evaluate());

// 	for(int i = 0; i < parsed.ncount; i++) {
// 		if(parsed.nodes[i]->type == MKNTYP(NODE_INNER, INODE_BINOP)) {
// 			printf("%d\n", ((NumberNode*)((BinOpNode*)parsed.nodes[i])->evaluate())->evaluate());
// 		}
// 	}
	
	return 0;
}
