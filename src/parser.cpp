// todo
/// parse &, &&, |
/// parse ()
/// parse commands & their params

#include "parser.hpp"
#include "lexer.hpp"
#include <cstdio>
#include <cstdlib>
#include <memory>

// we need to probably parse from inside out somehow
// that means first make all leaf nodes for the inner node x and then
// construct x

// todo write a function that constructs leaf nodes for inner node x

// todo pretty print function
// todo slight refactor, these funcs are stupid

int sortToks(Token *start, Token *end, int breaktok) {
	int curtok = -1, lastok = -1, skip = 0, i = 0;
	Token *current, *last = nullptr;

	for(; (end != nullptr && i < (end - start)) || (end == nullptr && current->type != breaktok); i++,i+=skip,skip=0, last=current) {
		current = &start[i];

		if(/*end&&*/current->type == TOK_LEFTPAREN) { skip = sortToks(&start[i+1], nullptr, TOK_RIGHTPAREN); continue; }

		if(last && last->type == TOK_NUMBER && current->type >= TOK_PLUS && current->type <= TOK_SLASH) {
			//printf("swap: %d %d\n", start[i-1].token, start[i-2].token);
			std::swap(start[i], start[i-1]);
		}
	}

	if(end == nullptr && breaktok) return i+1;
	return i;
}

std::pair<int, Node*> makeNode(Token *tokens, int i) {
	switch(tokens[i].type) {
	case TOK_PLUS: {
		auto [lskip, lhs] = makeNode(tokens, i+1);
		auto [rskip, rhs] = makeNode(tokens, i+2);
		return std::make_pair(lskip+rskip, new AdditionNode(lhs, rhs));
	} break;
	case TOK_MINUS: {
		auto [lskip, lhs] = makeNode(tokens, i+1);
		auto [rskip, rhs] = makeNode(tokens, i+2);
		return std::make_pair(lskip+rskip, new SubtractionNode(lhs, rhs));
	} break;
	case TOK_ASTERISK: {
		auto [lskip, lhs] = makeNode(tokens, i+1);
		auto [rskip, rhs] = makeNode(tokens, i+2);
		return std::make_pair(lskip+rskip, new MultiplicationNode(lhs, rhs));
	} break;
	case TOK_SLASH: {
		auto [lskip, lhs] = makeNode(tokens, i+1);
		auto [rskip, rhs] = makeNode(tokens, i+2);
		return std::make_pair(lskip+rskip, new DivisionNode(lhs, rhs));
	} break;
	case TOK_LEFTPAREN: {
		int j;
		ParenthesesNode *n = new ParenthesesNode();
		for(j = i+1; tokens[j].type != TOK_RIGHTPAREN; j++) {
			Node *node = makeNode(tokens, j).second;
			n->append(node);
		}

		return std::make_pair(j-i, n);
	} break;
	case TOK_NUMBER: {
		return std::make_pair(1, new NumberNode(tokens[i].value));
	} break;

//	case TOK_SWITCH:
	case TOK_ID: {
		return std::make_pair(1, new StringNode((char*)tokens[i].value));
	}
	
	default: return std::make_pair(0, nullptr);
	}
}

Parser::Parser(char *buffer, size_t length)
 : lexer(buffer, length)
{
}

void Parser::parse() {
	auto [noOfTokens, tokens] = lexer.lexBuffer();
	int i;

	//sortToks(&tokens[0], &tokens[noOfTokens], 0);

	for(i = 0; i < noOfTokens; i++) {
		printf("%d\n", tokens[i].type);
	}
}
