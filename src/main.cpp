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

#include <magic_enum.hpp>

#include "win.hh"
#include "interpreter.hh"

template<typename T>
void print_binary(T number) {
    if (number >> 1) print_binary(number >> 1);
    putc((number & 1) ? '1' : '0', stdout);
}

void prettyPrint(int level, Node *n) {
	if(n == nullptr) return;
	printf("[%s]", __FILE__);
	for(int l = -1; l < 2*level; l++) printf(" ");
	putc('(', stdout); print_binary<unsigned short>(n->type); printf(") ");
	auto [_1, _2] = n->stringify();
	printf("%s |%s|\n", _1, _2);

	if(n->type & 1) { // inner
		if((n->type & BARETYPE) == Node::Type::If) {
			IfNode *ifn = n;
			prettyPrint(level+1, ifn->condition);
			prettyPrint(level+1, ifn->sucess);
			if(ifn->failure != nullptr) prettyPrint(level+1, ifn->failure);
		}
		else if((n->type & BARETYPE) == Node::Type::For) {
			ForNode *fn = n;

			printf("[%s]", __FILE__);
			for(int l = -1; l < 2*(level+1); l++) printf(" ");
			printf("%c\n", fn->id);

			printf("[%s]", __FILE__);
			for(int l = -1; l < 2*(level+1); l++) printf(" ");
			printf("%s\n", magic_enum::enum_name(fn->forType).data());

			//prettyPrint(level+1, fn->opts);
			prettyPrint(level+1, fn->cond);
			prettyPrint(level+1, fn->loopBody);
		}
		else {
			if(n->type & WITHCHILDREN)
				for (Node *child : *n->children) prettyPrint(level+1, child);
			else {
				prettyPrint(level+1, n->lhs);
				prettyPrint(level+1, n->rhs);
			}
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
