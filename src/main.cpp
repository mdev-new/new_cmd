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
#include <cstring>

#include <ctime>

#include <unistd.h>

#include "win.hh"
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
	printf("%s |%s|\n", _1, _2);
	if(n->type & 1) { // inner
		InnerNode *nn = n;
		if(nn->type & WITHCHILDREN) {
			for (int x = 0; x < nn->childrenCount; x++) prettyPrint(level+1, nn->children[x]);
		} else {
			if((nn->type & BARETYPE) == IN(INODE_IF)) {
				IfNode *ifn = n;
				prettyPrint(level+1, ifn->condition);
				if(ifn->sucess != nullptr) prettyPrint(level+1, ifn->sucess);
				if(ifn->failure != nullptr) prettyPrint(level+1, ifn->failure);
			} else if((nn->type & BARETYPE) == IN(INODE_FOR)) {
				ForNode *fn = n;
				for (auto p : fn->params) {
					prettyPrint(level+1, p);
				}
			} else {
				prettyPrint(level+1, nn->lhs);
				prettyPrint(level+1, nn->rhs);
			}
		}
	} else { // leaf
		if((n->type & BARETYPE) == MKNTYP(NODE_LEAF, LNODE_CALL)) {
			CallNode *cln = n;
			for(int p = 0; p < cln->args.size(); p++) prettyPrint(level+1, cln->args[p]);
		}
	}
}

int main(int argc, char *argv[], char *envp[]) {
	setenv("mbat_version", "001al", true);
	if(argc < 2) {
		char line[512], path[4096];
		printf("%s> ", getcwd(path, 4096));
		while(fgets(line, sizeof line, stdin) != NULL) {
			Interpreter(line, strlen(line)).interpret();
			printf("%s> ", getcwd(path, 4096));
		}

		return 0;
	}

	FILE *f = fopen(argv[1], "r");
	if(f == nullptr) return -1;
	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	rewind(f);

	char *buffer = calloc(size, sizeof(char));
	fread(buffer, size, 1, f);

	Interpreter intp(buffer, size);

	for(Node *n : intp.nodes) prettyPrint(0, n);

	fprintf(stderr, "---\n");

	int retncode = 0;

	struct timespec begin, end;
	clock_gettime(CLOCK_REALTIME, &begin);

	retncode = intp.interpret();

	clock_gettime(CLOCK_REALTIME, &end);
	double timetaken = (end.tv_sec - begin.tv_sec) + (end.tv_nsec - begin.tv_nsec)*1e-6;
	fprintf(stderr, "---\n");
	fprintf(stderr, "Interpreting took: %.3f ms\n", timetaken);
	fprintf(stderr, "Root nodes per second: %.f\n", intp.nodes.size() * (1000./timetaken));

	//printf("%%hello%% = %s\n", getenv("hello"));

	free(buffer);
	fclose(f);
	return retncode;
}
