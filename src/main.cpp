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

#include <sys/time.h>

#include <unistd.h>

#include "win.hh"
#include "interpreter.hh"

void print_binary(unsigned int number) {
    if (number >> 1) print_binary(number >> 1);
    putc((number & 1) ? '1' : '0', stdout);
}

void prettyPrint(int level, Node *n) {
	//printf("TYPE: %d\n", n->type);
	printf("[%s]", __FILE__);
	for(int l = 0; l < 2*level; l++) printf(" ");
	//putc('(', stdout); print_binary(n->type); printf(") ");
	auto [_1, _2] = n->stringify();
	printf("%s |%s|\n", _1, _2);
	if(n->type & 1) { // inner
		InnerNode<Node*> *nn = (InnerNode<Node*> *)n;
		if(n->type & WITHCHILDREN) {
			for (int x = 0; x < nn->children->size(); x++) prettyPrint(level+1, (*nn->children)[x]);
		} else {
			if((n->type & BARETYPE) == NodeType::If) {
				IfNode *ifn = n;
				prettyPrint(level+1, ifn->condition);
				if(ifn->sucess != nullptr) prettyPrint(level+1, ifn->sucess);
				if(ifn->failure != nullptr) prettyPrint(level+1, ifn->failure);
			} else if((n->type & BARETYPE) == NodeType::For) {
				ForNode *fn = n;
				// for (auto p : fn->params) {
				// 	prettyPrint(level+1, p);
				// }
			} else {
				prettyPrint(level+1, nn->lhs);
				prettyPrint(level+1, nn->rhs);
			}
		}
	} else { // leaf
		if((n->type & BARETYPE) == NodeType::Call) {
			CallNode *cln = n;
			for(int p = 0; p < cln->children->size(); p++) prettyPrint(level+1, (*cln->children)[p]);
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

	struct timeval tval_before, tval_after, tval_result;

	gettimeofday(&tval_before, NULL);
	FILE *f = fopen(argv[1], "r");
	if(f == nullptr) return -1;
	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	rewind(f);

	char *buffer = calloc(size, sizeof(char));
	fread(buffer, size, 1, f);

	gettimeofday(&tval_after, NULL);
	timersub(&tval_after, &tval_before, &tval_result);
	fprintf(stderr, "[%s] Reading took:\t%ld.%06ld\n", __FILE__, (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);

	gettimeofday(&tval_before, NULL);
	Interpreter intp(buffer, size);
	gettimeofday(&tval_after, NULL);
	timersub(&tval_after, &tval_before, &tval_result);
	fprintf(stderr, "[%s] Parsing took:\t%ld.%06ld\n", __FILE__, (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);

	//for(Node *n : intp.nodes) prettyPrint(0, n);

	int retncode = 0;

	gettimeofday(&tval_before, NULL);

	retncode = intp.interpret();

	gettimeofday(&tval_after, NULL);
	timersub(&tval_after, &tval_before, &tval_result);

	double timetaken = (double)tval_result.tv_sec + (double)tval_result.tv_usec / 1000000.;

	fprintf(stderr, "[%s] Interpreter took:%f\n", __FILE__, timetaken);
	fprintf(stderr, "[%s] Root commands/s:\t%.f\n", __FILE__, intp.nodes.size() / timetaken);

	//printf("%%hello%% = %s\n", getenv("hello"));

	free(buffer);
	fclose(f);
	return retncode;
}
