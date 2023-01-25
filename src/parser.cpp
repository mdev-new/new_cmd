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

#include <utility>

//#include "patterns/patterns.hpp"

#define TCAST(type, val) ((type)val)

// stackoverflow saved my ass
template <typename t> void move(std::vector<t>& v, size_t oldIndex, size_t newIndex) {
	if (oldIndex > newIndex) std::rotate(v.rend() - oldIndex - 1, v.rend() - oldIndex, v.rend() - newIndex);
	else std::rotate(v.begin() + oldIndex, v.begin() + oldIndex + 1, v.begin() + newIndex + 1);
}

static std::unordered_set<int> command_terminators = {TokenType::Pipe, TokenType::And, TokenType::PipeIn, TokenType::PipeOut, TokenType::Space, TokenType::Eof};

// stage 2 lexer basically
int SortTokens(std::vector<Token> &tokens, int start, int breaktok) {
	Token current = tokens[start], last = current;
	long i = start, skip = 0, parenStart = 0;

	for(; breaktok? (current.type != breaktok && i < tokens.size()-1) : (i < tokens.size()-1); i+=skip, skip=1) {
		current = tokens[i];

// 		match(tokens[i-1].type, tokens[i].type, tokens[i+1].type) (
// 			pattern(_, TokenType::Leftparen, _) = [&] {
// 				parenStart = 0;
// 				skip = SortTokens(tokens, i+1, TokenType::Rightparen);
// 				last = tokens[i+skip-1];
// //				continue;
// 			},
// 			pattern(_, TokenType::Equals, TokenType::Equals) = [&] {
// 				tokens.erase(tokens.begin() + i+1);
// 				tokens[i] = (Token){TokenType::Dequal, 0, 0};
// 				std::swap(tokens[i], tokens[i-1]);
// 				continue;
// 			},
// 			pattern(TokenType::Rightparen,  >= TokenType::Plus && _ <= TokenType::Slash, _)
// 		);

		if(current.type == TokenType::Leftparen) {
			parenStart = i;
			skip = SortTokens(tokens, i+1, TokenType::Rightparen);
			last = tokens[i+skip-1];
			continue;
		}

		if(last.type == TokenType::Rightparen && current.type >= TokenType::Plus && current.type <= TokenType::Slash) {
			move(tokens, i, parenStart);
			parenStart = 0;
		}

		if((last.type == TokenType::Number) && current.type >= TokenType::Plus && current.type <= TokenType::Slash) {
			std::swap(tokens[i], tokens[i-1]);
		}

		if(current.type == TokenType::Equals && tokens[i+1].type == TokenType::Equals) {
			tokens.erase(tokens.begin() + i+1);
			tokens[i] = (Token){TokenType::Dequal, 0, 0};
			std::swap(tokens[i], tokens[i-1]);
			continue;
		}

		switch(current.type) {
		case TokenType::Gtr:
		case TokenType::Lss:
		case TokenType::Geq:
		case TokenType::Leq:
		case TokenType::Equ:
		case TokenType::Neq:
			std::swap(tokens[i], tokens[i-1]);
		default: break;
		}

		// todo reduce multiple consecutive slashes
		// todo move rhs to the front (incase thi happens =xyz0, =0xyz would be easier to parse)
		if(current.type == TokenType::Equals) std::swap(tokens[i], tokens[i-1]);

		if ((last.type == TokenType::Slash || last.type == TokenType::Minus) && (current.type == TokenType::String || current.type == TokenType::Id)) {
			char pref = (last.type == TokenType::Slash)? '/' : '-';
			tokens.erase(tokens.begin() + i-1);
			tokens[i-1] = current = (Token){TokenType::Switch, current.value, pref};
		}

		if (last.type == TokenType::Colon && (current.type == TokenType::String || current.type == TokenType::Id)) {
			char pref = ':';
			tokens.erase(tokens.begin() + i-1);
			tokens[i-1] = current = (Token){TokenType::Label, current.value, pref};
		}

		last = current;
	}

	return i;
}

std::pair<int, Node*> makeNode(std::vector<Token> &tokens, int i, int level) {
	switch(tokens[i].type) {
	case TokenType::Plus: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new AdditionNode(lhs, rhs));
	}

	case TokenType::Minus: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new SubtractionNode(lhs, rhs));
	}

	case TokenType::Asterisk: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new MultiplicationNode(lhs, rhs));
	}

	case TokenType::Slash: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new DivisionNode(lhs, rhs));
	}

	case TokenType::Leftparen: {
		int j, skip = 1;
		ParenthesesNode *n = new ParenthesesNode();
		for(j = i+1; tokens[j].type != TokenType::Rightparen && tokens[j].type != TokenType::Eof; j+=skip, skip=1) {
			auto [ss, nn] = makeNode(tokens, j, 0);
			n->children->push_back((decltype(n->lhs))nn);
			skip = ss;
		}

		return std::make_pair(j-i+1, n);
	}

	case TokenType::Number: {
		return std::make_pair(1, new NumberNode(tokens[i].value));
	}

	case TokenType::Equals: {
		// todo support multiple assignments (for example a=b=c=0)
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new AssignNode(lhs, rhs));
	}

	case TokenType::Lss: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::LSS));
	}

	case TokenType::Leq: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::LEQ));
	}

	case TokenType::Gtr: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::GTR));
	}

	case TokenType::Geq: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::GEQ));
	}

	case TokenType::Equ: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::EQU));
	}

	case TokenType::Neq: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::NEQ));
	}

	case TokenType::Dequal: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::STRING));
	}

	case TokenType::For: {
		ForNode::ForType fortyp = ForNode::ForType::FILES;
		ParenthesesNode *cond;
		Node *body, *current;
		char id;

		int _i = i+1, skip = 1;
		for(; command_terminators.count(tokens[_i].type) == 0 && _i < tokens.size()-1; _i+=skip, skip = 1) {
			std::tie(skip, current) = makeNode(tokens, _i, level+1);
			if(current == nullptr) { 
				delete current;
				continue;
			}

			switch(current->type & BARETYPE) {
			case NodeType::Switch: {
				SwitchNode *sw = current;
				switch(sw->str[0] | 0x20) {
				case 'l': {
					fortyp = ForNode::ForType::NUMBERS;
					break;
				}
				case 'f': {
					break;
				}
				case 'd': {
					fortyp = ForNode::ForType::FOLDERS;
					break;
				}
				case 'r': {
					fortyp = ForNode::ForType::FILESROOTED;
					break;
				}
				}
				break;
			}
			default: break;
			}
		}

		return std::make_pair(_i-i, new ForNode(fortyp, id, cond, body));
	}

	case TokenType::If: {
		int _i = i+1, skip = 1;

		CompareNode *cmp;
		Node *sucessRoot = nullptr, *failureRoot = nullptr;
		bool invert = false, caseInsensitive = false;
		bool elseEncountered = false;

		for(; command_terminators.count(tokens[_i].type) == 0 && _i < tokens.size()-1; _i+=skip, skip = 1) {
			if(tokens[i].type == TokenType::Else && sucessRoot != nullptr) {
				elseEncountered = true;
				continue;
			}
			auto [skp, nod] = makeNode(tokens, _i, level+1);
			if(nod != nullptr) {
				switch(nod->type & BARETYPE) {
				case NodeType::Compare: {
					cmp = nod;
					break;
				}
				case NodeType::Switch: {
					SwitchNode *sw = nod;
					if(strcasecmp(sw->str, "i") == 0) caseInsensitive = true;
					else if(strcasecmp(sw->str, "not") == 0) invert = true;
					break;
				}
				//case IN(INODE_PARENTHESES):
				case NodeType::Call: {
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

	case TokenType::String: return std::make_pair(1, new StringNode((char*)tokens[i].value));
	case TokenType::Switch: return std::make_pair(1, new SwitchNode((char*)tokens[i].value, tokens[i].additionalData));
	case TokenType::Label: return std::make_pair(1, new LabelNode((char*)tokens[i].value, i-1));

	case TokenType::Id:
		if(level != 0) { return std::make_pair(1, new IdNode((char*)tokens[i].value)); break; }
		// fallthrough
	case TokenType::BuiltIn: {
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

		if(tokens.size() >= i-1) return std::make_pair(_i-i, new CallNode((char*)tokens[i].value, args, tokens[i-1].type == TokenType::At));
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
