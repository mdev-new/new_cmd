// TODO:
// interpreter for files (WIP, obviously)
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

#include "standard.h"
#include <magic_enum.hpp>

#include "utilities/file_utils.h"

#include "interpreter/interpreter.hh"

template<typename T>
void print_binary(T number, FILE *out) {
    if (number >> 1) print_binary(number >> 1);
    putc((number & 1) ? '1' : '0', out);
}

void prettyPrint(int level, Node *n) {
	if(n == nullptr) return;
	fprintf(stderr, "[%s]", __FILE__);
	for(int l = -1; l < 2*level; l++) fprintf(stderr, " ");
	auto [_1, _2] = n->stringify();
	fprintf(stderr, "%s |%s| ", _1, _2);

#if 0
	putc('(', stderr); print_binary<unsigned short>(n->type, stderr); fprintf(stderr, ") \n");
#else
	fprintf(stderr, "\n");
#endif

	if(n->type & 1) { // inner
		if((n->type & BARETYPE) == Node::Type::If) {
			IfNode *ifn = n;
			prettyPrint(level+1, ifn->condition);
			prettyPrint(level+1, ifn->sucess);
			if(ifn->failure != nullptr) prettyPrint(level+1, ifn->failure);
		}
		else if((n->type & BARETYPE) == Node::Type::For) {
			ForNode *fn = n;

			fprintf(stderr, "[%s]", __FILE__);
			for(int l = -1; l < 2*(level+1); l++) fprintf(stderr, " ");
			fprintf(stderr, "%c\n", fn->id);

			fprintf(stderr, "[%s]", __FILE__);
			for(int l = -1; l < 2*(level+1); l++) fprintf(stderr, " ");
			fprintf(stderr, "%s\n", magic_enum::enum_name(fn->forType).data());

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
		auto s = InterpreterState {
			.echo = true,
			.filepos = 0
		};
		printf("(new_cmd) %s> ", getcwd(path, 4096));
		while(fgets(line, sizeof line, stdin) != NULL) {
			s.filepos = 0;
			Interpreter(line, strlen(line), &s).interpret();
			printf("(new_cmd) %s> ", getcwd(path, sizeof path));
		}

		return 0;
	}

	struct timeval tval_before, tval_after, tval_result;

	gettimeofday(&tval_before, NULL);
	FILE *f = fopen(argv[1], "r");
	if(f == nullptr) return -1;
	size_t size = get_file_size(f);

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

//#define VALGRIND_BUILD
#ifndef VALGRIND_BUILD
	for(Node *n : intp.nodes) prettyPrint(0, n);
#endif

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
