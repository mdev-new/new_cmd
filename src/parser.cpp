// todo
/// parse &, &&, |
/// parse ()
/// parse commands & their params

#include "parser.hpp"
#include "lexer.hpp"
#include <cstdio>
#include <cstdlib>
#include <memory>

// TODO sort operations; smth like
// a+b => +ab
// a+(b/c)*d => *d+a(/bc)

int SortTokens(Token *start, size_t len, int breaktok) {
	Token *current = start, *last = nullptr;
	long i, skip;

	for(i = skip = 0; ((breaktok != 0)? (current->type != breaktok) : (i < len)); i+=1+skip, skip=0) {
		current = &start[i];
		if(current->type == TOK_LEFTPAREN) {
			skip = SortTokens(&start[i+1], 0, TOK_RIGHTPAREN);
			continue;
		}

		if(last && (last->type == TOK_NUMBER) && current->type >= TOK_PLUS && current->type <= TOK_SLASH) {
			std::swap(start[i], start[i-1]);
		}

		last = current;
	}

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
	int i, skip;

	SortTokens(tokens, noOfTokens, 0);

	for(i = skip = 0; i < noOfTokens; i+=1+skip, skip = 0) {
		auto [_skip, _node] = makeNode(tokens, i);
		if(_skip != _node != 0) {
			this->nodes.push_back(_node);
			skip = _skip;
		}
	}
}
