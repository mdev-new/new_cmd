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

int evalExpr(Node *root) {
	switch(root->type) {
	case MKNTYP(NODE_LEAF, LNODE_NUMBER): return ((NumberNode*)root)->evaluate();
	case MKNTYP(NODE_INNER, INODE_PARENTHESES): return ((ParenthesesNode*)root)->evaluate();
	case MKNTYP(NODE_INNER, INODE_BINOP): return ((NumberNode*)((BinOpNode*)root)->evaluate())->evaluate();
	}
}

int sortToks(Token *start, Token *end, int breaktok) {
	int curtok = -1, lastok = -1, skip = 0, i = 0;
	Token *current, *last = nullptr;

	for(; (end != nullptr && i < (end - start)) || (end == nullptr && current->token != breaktok); i++,i+=skip,skip=0, last=current) {
		current = &start[i];

		if(/*end&&*/current->token == TOK_OPENING_PAREN) { skip = sortToks(&start[i+1], nullptr, TOK_CLOSING_PAREN); continue; }

		if(last && last->token == TOK_NUMBER && current->token >= TOK_PLUS && current->token <= TOK_DIVIDE) {
			//printf("swap: %d %d\n", start[i-1].token, start[i-2].token);
			std::swap(start[i], start[i-1]);
		}
	}

	if(end == nullptr && breaktok) return i+1;
	return i;
}

std::pair<int, Node*> makeNode(Token *tokens, int i) {
	switch(tokens[i].token) {
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
	case TOK_MULTIPLY: {
		auto [lskip, lhs] = makeNode(tokens, i+1);
		auto [rskip, rhs] = makeNode(tokens, i+2);
		return std::make_pair(lskip+rskip, new MultiplicationNode(lhs, rhs));
	} break;
	case TOK_DIVIDE: {
		auto [lskip, lhs] = makeNode(tokens, i+1);
		auto [rskip, rhs] = makeNode(tokens, i+2);
		return std::make_pair(lskip+rskip, new DivisionNode(lhs, rhs));
	} break;
	case TOK_OPENING_PAREN: {
		int j;
		ParenthesesNode *n = new ParenthesesNode();
		for(j = i+1; tokens[j].token != TOK_CLOSING_PAREN; j++) {
			Node *node = makeNode(tokens, j).second;
			n->append(node);
		}

		return std::make_pair(j-i, n);
	} break;
	case TOK_NUMBER: {
		return std::make_pair(1, new NumberNode(tokens[i].value));
	} break;

	case TOK_SWITCH:
	case TOK_UNDEFINED: {
		return std::make_pair(1, new StringNode((char*)tokens[i].value));
	}
	
	default: return std::make_pair(0, nullptr);
	}
}

class Parser {
private:
public:
	ParsedFile parse(const LexedFile &lexedFile) {

	int treshold = 64, created = 0;
	Node **buf = malloc(treshold*sizeof(Node*));

	sortToks(&lexedFile.tokens[0], &lexedFile.tokens[lexedFile.noOfTokens-1], -1);

	Token *current = nullptr, *last = nullptr;
	for(int i = 0, skip = 0; i < lexedFile.noOfTokens; i=i+1+skip, skip=0, last=current) {
		if(created >= treshold) buf = realloc(buf, (treshold += 64)*sizeof(Node*));
		current = &lexedFile.tokens[i];

		auto [_skip, node] = makeNode(lexedFile.tokens, i);
		if(_skip != node != NULL) buf[created++] = node;
		skip = _skip;
	}

	buf = realloc(buf, created*sizeof(Node*));
	return (ParsedFile){buf, created};
	
	}
};

ParsedFile parse(LexedFile &lexedFile) {
	Parser parser;
	return parser.parse(lexedFile);
}
