#include "parser.hpp"
#include "lexer.hpp"
#include <cstdio>
#include <cstdlib>

// makes it easier to track internal state, and ability to use `this` instead of pointers
// this class is not exposed
class Parser {
private:
public:
	Node parse(const LexedFile &lexedFile) {
		int nodesCreated = 0, bufsize = 128*sizeof(Node);
		Node *prgmNodeBuf = malloc(bufsize);

		Token prev = {-1}, current, next = {-1};

		for(int i = 0; i < lexedFile.sizeOfTokens; i++) {
			if(nodesCreated > 127) prgmNodeBuf = realloc(prgmNodeBuf, bufsize += 128*sizeof(Node));
			current = lexedFile.tokens[i];
			next = lexedFile.tokens[i+1];

			if(current.token == TOK_PLUS) {	
				Node *children = malloc(2*sizeof(Node));
				children[0] = {nullptr, 0, Value((int)prev.value), NODE_NUMBER};
				children[1] = {nullptr, 0, Value((int)next.value), NODE_NUMBER};
				prgmNodeBuf[nodesCreated++] = {children, 2, Value(), NODE_PLUS_NOVAL };
			}

			//if(current.token == TOK_UNDEFINED) {
				//if((prev.token == TOK_PERCENT && next.token == TOK_PERCENT) || (prev.token == TOK_LOGICALNOT && next.token == TOK_LOGICALNOT)) prgmNodeBuf[nodesCreated++] = {nullptr, 0, Value((char *)current.value), .valueType = NODE_ENVVAR};
			//}

			prev = current;
		}

		return { .children=prgmNodeBuf, .noOfChildren = nodesCreated, .value = Value(), .valueType = NODE_NOVAL };
	}
};

ParsedFile parse(LexedFile &lexedFile) {
	Parser parser;
	return {.rootNode = parser.parse(lexedFile) };
}
