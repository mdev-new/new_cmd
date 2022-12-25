// TODO:
// basic interpreter for files
// repl support
// fix memory leaks (free tokens, nodes, etc)

// far targets:
// caching
// multithreading
// local caching of environment vars?
// JIT compilation?

// goals
// 1. speed
// 2. speed
// 3. also speed

//#define WIN32_LEAN_AND_MEAN
//#include <Windows.h>
#include <cstdio>
#include <cstdlib>

#include "parser.hpp"
#include "interpreter.hh"

void prettyPrint(int level, Node *n) {
	printf("TYPE: %d\n", n->type);
	for(int l = 0; l < level*2; l++) printf(" ");
	if(n->type & 1) { // inner
		InnerNode *nn = n;
		if(nn->type & WITHCHILDREN) {
			printf("Inner: %s\n", nn->stringify().first);
			for (int x = 0; x < nn->childrenCount; x++) prettyPrint(level+1, nn->children[x]);
		} else {
			printf("Inner: %s\n", nn->stringify().first);
			prettyPrint(level+1, nn->lhs);
			prettyPrint(level+1, nn->rhs);
		}
	} else { // leaf
		printf("Leaf: %s\n", n->stringify().first);
		if(n->type == (MKNTYP(NODE_LEAF, LNODE_CALL) & BARETYPE)) {
			CallNode *cln = n;
			for(int p = 0; p < cln->args.size(); p++) prettyPrint(level+1, cln->args[p]);
		}
	}
}

int main(int argc, char *argv[]) {
	if(argc < 2) return 0;

	setenv("mbat_version", "001al", true);

	FILE *f = fopen(argv[1], "r");
	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	rewind(f);

	char *buffer = malloc(size+1);
	fread(buffer, size, 1, f);

	Interpreter intp(buffer, size);
	int retncode = intp.interpret();

	for(Node *n : intp.nodes) {
		prettyPrint(0, n);
	}

	free(buffer);
	fclose(f);
	return retncode;
}
