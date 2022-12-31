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

#include "win.hh"
#include "parser.hpp"
#include "interpreter.hh"

void print_binary(unsigned int number) {
    if (number >> 1) print_binary(number >> 1);
    putc((number & 1) ? '1' : '0', stdout);
}

void prettyPrint(int level, Node *n) {
	//printf("TYPE: %d\n", n->type);
	for(int l = 0; l < 2*level; l++) printf(" ");
	//putc('(', stdout); print_binary(n->type); printf(") ");
	auto [_1, _2] = n->stringify();
	if(n->type & 1) { // inner
		InnerNode *nn = n;
		if(nn->type & WITHCHILDREN) {
			printf("%s |%s|\n", _1, _2);
			for (int x = 0; x < nn->childrenCount; x++) prettyPrint(level+1, nn->children[x]);
		} else {
			printf("%s |%s|\n", _1, _2);
			prettyPrint(level+1, nn->lhs);
			prettyPrint(level+1, nn->rhs);
		}
	} else { // leaf
		printf("%s |%s|\n", _1, _2);
		if((n->type & BARETYPE) == (MKNTYP(NODE_LEAF, LNODE_CALL) & BARETYPE)) {
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
	buffer[size] = 0;

	Interpreter intp(buffer, size);

	for(Node *n : intp.nodes) prettyPrint(0, n);

	struct timespec begin, end; 
	clock_gettime(CLOCK_REALTIME, &begin);

	int retncode = intp.interpret();

	clock_gettime(CLOCK_REALTIME, &end);
	printf("Interpreting took: %.3f ms\n", (end.tv_sec - begin.tv_sec) + (end.tv_nsec - begin.tv_nsec)*1e-6);

	//printf("%%hello%% = %s\n", getenv("hello"));

	free(buffer);
	fclose(f);
	return retncode;
}
