// todo
/// parse &, &&, |
/// parse ()
/// parse commands & their params

#include "parser.hpp"
#include "lexer.hpp"
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <algorithm>
#include <unordered_set>
#include <cstring>
#include "shared.hh"
#include "nodes.hh"

#define TCAST(type, val) ((type)val)

// stackoverflow saved my ass
template <typename t> void move(std::vector<t>& v, size_t oldIndex, size_t newIndex) {
	if (oldIndex > newIndex) std::rotate(v.rend() - oldIndex - 1, v.rend() - oldIndex, v.rend() - newIndex);
	else std::rotate(v.begin() + oldIndex, v.begin() + oldIndex + 1, v.begin() + newIndex + 1);
}

static std::unordered_set<int> command_terminators = {TOK_PIPE, TOK_AND, TOK_PIPEIN, TOK_PIPEOUT, TOK_SPACE, TOK_EOF};

// stage 2 lexer basically
int SortTokens(std::vector<Token> &tokens, int start, int breaktok) {
	Token current = tokens[start], last = current;
	long i = start, skip = 0, parenStart = 0;

	for(; breaktok? (current.type != breaktok && i < tokens.size()-1) : (i < tokens.size()-1); i+=skip, skip=1) {
		current = tokens[i];

		if(current.type == TOK_LEFTPAREN) {
			parenStart = i;
			skip = SortTokens(tokens, i+1, TOK_RIGHTPAREN);
			last = tokens[i+skip-1];
			continue;
		}

		if(last.type == TOK_RIGHTPAREN && current.type >= TOK_PLUS && current.type <= TOK_SLASH) {
			move(tokens, i, parenStart);
			parenStart = 0;
		}

		if((last.type == TOK_NUMBER) && current.type >= TOK_PLUS && current.type <= TOK_SLASH) {
			std::swap(tokens[i], tokens[i-1]);
		}

		if(current.type == TOK_EQUALS && tokens[i+1].type == TOK_EQUALS) {
			tokens.erase(tokens.begin() + i+1);
			tokens[i] = (Token){TOK_DEQUAL, 0, 0};
			std::swap(tokens[i], tokens[i-1]);
			continue;
		}

		switch(current.type) {
		case TOK_GTR:
		case TOK_LSS:
		case TOK_GEQ:
		case TOK_LEQ:
		case TOK_EQU:
		case TOK_NEQ:
			std::swap(tokens[i], tokens[i-1]);
		default: break;
		}

		// todo reduce multiple consecutive slashes
		// todo move rhs to the front (incase thi happens =xyz0, =0xyz would be easier to parse)
		if(current.type == TOK_EQUALS) std::swap(tokens[i], tokens[i-1]);

		if ((last.type == TOK_SLASH || last.type == TOK_MINUS) && (current.type == TOK_STRING || current.type == TOK_ID)) {
			char pref = (last.type == TOK_SLASH)? '/' : '-';
			tokens.erase(tokens.begin() + i-1);
			tokens[i-1] = current = (Token){TOK_SWITCH, current.value, pref};
		}

		if (last.type == TOK_COLON && (current.type == TOK_STRING || current.type == TOK_ID)) {
			char pref = ':';
			tokens.erase(tokens.begin() + i-1);
			tokens[i-1] = current = (Token){TOK_LABEL, current.value, pref};
		}

		last = current;
	}

	return i;
}

std::pair<int, Node*> makeNode(std::vector<Token> tokens, int i, int level) {
	switch(tokens[i].type) {
	case TOK_PLUS: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new AdditionNode(lhs, rhs));
	}

	case TOK_MINUS: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new SubtractionNode(lhs, rhs));
	}

	case TOK_ASTERISK: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new MultiplicationNode(lhs, rhs));
	}

	case TOK_SLASH: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new DivisionNode(lhs, rhs));
	}

	case TOK_LEFTPAREN: {
		int j, skip = 1;
		ParenthesesNode *n = new ParenthesesNode();
		for(j = i+1; tokens[j].type != TOK_RIGHTPAREN && tokens[j].type != TOK_EOF; j+=skip, skip=1) {
			auto [ss, nn] = makeNode(tokens, j, 0);
			n->append(nn);
			skip = ss;
		}

		return std::make_pair(j-i+1, n);
	}

	case TOK_NUMBER: {
		return std::make_pair(1, new NumberNode(tokens[i].value));
	}

	case TOK_EQUALS: {
		// todo support multiple assignments (for example a=b=c=0)
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new AssignNode(lhs, rhs));
	}

	case TOK_LSS: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::LSS));
	}

	case TOK_LEQ: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::LEQ));
	}

	case TOK_GTR: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::GTR));
	}

	case TOK_GEQ: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::GEQ));
	}

	case TOK_EQU: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::EQU));
	}

	case TOK_NEQ: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::NEQ));
	}

	case TOK_DEQUAL: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::STRING));
	}

	case TOK_FOR: {
		std::vector<Node*> args;
		int s = 1, x = 1;
		auto [skip1, node1] = makeNode(tokens, i+x, level+1);
		args.push_back(node1);
		s += skip1;
		while(args.back()->type != LN(LNODE_CALL) && args.back()->type != IN(INODE_PARENTHESES)) {
			auto [skip, node] = makeNode(tokens, i+x, level+1);
			s += skip;
			args.push_back(node);
			x++;
		}

		return std::make_pair(s, new ForNode(args));
	}

	case TOK_IF: {
		int _i = i+1, skip = 1;

		CompareNode *cmp;
		Node *sucessRoot = nullptr, *failureRoot = nullptr;
		bool invert = false, caseInsensitive = false;
		bool elseEncountered = false;

		for(; command_terminators.count(tokens[_i].type) == 0 && _i < tokens.size()-1; _i+=skip, skip = 1) {
			if(tokens[i].type == TOK_ELSE && sucessRoot != nullptr) {
				elseEncountered = true;
				continue;
			}
			auto [skp, nod] = makeNode(tokens, _i, level+1);
			if(nod != nullptr) {
				switch(nod->type & BARETYPE) {
				case IN(INODE_COMPARE): {
					cmp = nod;
					break;
				}
				case LN(LNODE_SWITCH): {
					SwitchNode *sw = nod;
					if(strcasecmp(sw->str, "i") == 0) caseInsensitive = true;
					else if(strcasecmp(sw->str, "not") == 0) invert = true;
					break;
				}
				//case IN(INODE_PARENTHESES):
				case LN(LNODE_CALL): {
					//printf("%s\n", nod->stringify().first);
					if(elseEncountered) failureRoot = nod;
					else sucessRoot = nod;
					break;
				}
				default: break;
				}
			} else delete nod;
		}

		return std::make_pair(_i-i, new IfNode(cmp, sucessRoot, failureRoot, invert, caseInsensitive));
	}

	case TOK_STRING: return std::make_pair(1, new StringNode((char*)tokens[i].value));
	case TOK_SWITCH: return std::make_pair(1, new SwitchNode((char*)tokens[i].value, tokens[i].additionalData));
	case TOK_LABEL: return std::make_pair(1, new LabelNode((char*)tokens[i].value, i-1));

	case TOK_ID:
		if(level != 0) { return std::make_pair(1, new IdNode((char*)tokens[i].value)); break; }
		// fallthrough
	case TOK_BUILTIN: {
		int _i = i+1, skip = 1;

		std::vector<Node *> args;

		bool isCall = strcasecmp(tokens[i].value, "call") == 0 || strcasecmp(tokens[i].value, "start") == 0; // lets avoid headaches caused by reparsing that line later on :)

		for(; command_terminators.count(tokens[_i].type) == 0 && _i < tokens.size(); _i+=skip, skip = 1) {
			auto [skp, nod] = makeNode(tokens, _i, isCall? 0 : 1);
			if(nod != nullptr) {
				args.push_back(nod);
				skip = skp;
			}
			else delete nod;
		}

		if(tokens.size() >= i-1) return std::make_pair(_i-i, new CallNode((char*)tokens[i].value, args, tokens[i-1].type == TOK_AT));
		else return std::make_pair(_i-i, new CallNode((char*)tokens[i].value, args));
	}

	default: return std::make_pair(1, nullptr);
	}
}

Parser::Parser(char *buffer, size_t length)
 : lexer(buffer, length)
{
	this->tokens = lexer.lexBuffer();
}

void Parser::parse() {
	int i, skip;

	SortTokens(tokens, 0, 0);

	//for(int x = 0; x < tokens.size()-1; x++) printf("%d\n", tokens[x].type);

	for(i = 0, skip = 1; i < tokens.size()-1; i+=skip, skip = 1) {
		auto [_skip, _node] = makeNode(tokens, i, 0);
		if(_node == nullptr) continue;
		this->nodes.push_back(_node);
		skip = _skip;
	}
}
