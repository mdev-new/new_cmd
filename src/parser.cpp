#include "parser.hpp"
#include "lexer.hpp"
#include <cstdio>
#include <cstdlib>
#include <memory>

// makes it easier to track internal state, and ability to use `this` instead of pointers
// this class is not exposed
class Parser {
private:
public:
	Node* parse(const LexedFile &lexedFile) {
		Token current;

		for(int i = 0; i < lexedFile.noOfTokens; i++) {
			current = lexedFile.tokens[i];
		}

	}
};

ParsedFile parse(LexedFile &lexedFile) {
	Parser parser;
	return {.rootNode = parser.parse(lexedFile) };
}
