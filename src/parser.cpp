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

static std::unordered_set<int> command_terminators = {Token::Type::Pipe, Token::Type::And, Token::Type::PipeIn, Token::Type::PipeOut, Token::Type::Space, Token::Type::Eof};

// stage 2 lexer basically
int SortTokens(std::vector<Token> &tokens, int start, int breaktok) {
	Token current = tokens[start], last = current;
	long i = start, skip = 0, parenStart = 0;

	for(; breaktok? (current.type != breaktok && i < tokens.size()-1) : (i < tokens.size()-1); i+=skip, skip=1) {
		current = tokens[i];

// 		match(tokens[i-1].type, tokens[i].type, tokens[i+1].type) (
// 			pattern(_, Token::Type::Leftparen, _) = [&] {
// 				parenStart = 0;
// 				skip = SortTokens(tokens, i+1, Token::Type::Rightparen);
// 				last = tokens[i+skip-1];
// //				continue;
// 			},
// 			pattern(_, Token::Type::Equals, Token::Type::Equals) = [&] {
// 				tokens.erase(tokens.begin() + i+1);
// 				tokens[i] = (Token){Token::Type::Dequal, 0, 0};
// 				std::swap(tokens[i], tokens[i-1]);
// 				continue;
// 			},
// 			pattern(Token::Type::Rightparen,  >= Token::Type::Plus && _ <= Token::Type::Slash, _)
// 		);

		if(current.type == Token::Type::Leftparen) {
			parenStart = i;
			skip = SortTokens(tokens, i+1, Token::Type::Rightparen);
			last = tokens[i+skip-1];
			continue;
		}

		if(last.type == Token::Type::Rightparen && current.type >= Token::Type::Plus && current.type <= Token::Type::Slash) {
			move(tokens, i, parenStart);
			parenStart = 0;
		}

		if((last.type == Token::Type::Number) && current.type >= Token::Type::Plus && current.type <= Token::Type::Slash) {
			std::swap(tokens[i], tokens[i-1]);
		}

		if(current.type == Token::Type::Equals && tokens[i+1].type == Token::Type::Equals) {
			tokens.erase(tokens.begin() + i+1);
			tokens[i] = (Token){Token::Type::Dequal, 0, 0};
			std::swap(tokens[i], tokens[i-1]);
			continue;
		}

		switch(current.type) {
		case Token::Type::Gtr:
		case Token::Type::Lss:
		case Token::Type::Geq:
		case Token::Type::Leq:
		case Token::Type::Equ:
		case Token::Type::Neq:
			std::swap(tokens[i], tokens[i-1]);
		default: break;
		}

		// todo reduce multiple consecutive slashes
		// todo move rhs to the front (incase thi happens =xyz0, =0xyz would be easier to parse)
		if(current.type == Token::Type::Equals) std::swap(tokens[i], tokens[i-1]);

		if ((last.type == Token::Type::Slash || last.type == Token::Type::Minus) && (current.type == Token::Type::String || current.type == Token::Type::Id)) {
			char pref = (last.type == Token::Type::Slash)? '/' : '-';
			tokens.erase(tokens.begin() + i-1);
			tokens[i-1] = current = (Token){Token::Type::Switch, current.value, pref};
		}

		if (last.type == Token::Type::Colon && (current.type == Token::Type::String || current.type == Token::Type::Id)) {
			char pref = ':';
			tokens.erase(tokens.begin() + i-1);
			tokens[i-1] = current = (Token){Token::Type::Label, current.value, pref};
		}

		last = current;
	}

	return i;
}

std::pair<int, Node*> makeNode(std::vector<Token> &tokens, int i, int level) {
	switch(tokens[i].type) {
	case Token::Type::Plus: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new AdditionNode(lhs, rhs));
	}

	case Token::Type::Minus: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new SubtractionNode(lhs, rhs));
	}

	case Token::Type::Asterisk: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new MultiplicationNode(lhs, rhs));
	}

	case Token::Type::Slash: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new DivisionNode(lhs, rhs));
	}

	case Token::Type::Leftparen: {
		int j, skip = 1;
		ParenthesesNode *n = new ParenthesesNode();
		for(j = i+1; tokens[j].type != Token::Type::Rightparen && tokens[j].type != Token::Type::Eof; j+=skip, skip=1) {
			auto [ss, nn] = makeNode(tokens, j, 0);
			n->children->push_back((decltype(n->lhs))nn);
			skip = ss;
		}

		return std::make_pair(j-i+1, n);
	}

	case Token::Type::Number: {
		return std::make_pair(1, new NumberNode(tokens[i].value));
	}

	case Token::Type::Equals: {
		// todo support multiple assignments (for example a=b=c=0)
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new AssignNode(lhs, rhs));
	}

	case Token::Type::Lss: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::LSS));
	}

	case Token::Type::Leq: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::LEQ));
	}

	case Token::Type::Gtr: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::GTR));
	}

	case Token::Type::Geq: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::GEQ));
	}

	case Token::Type::Equ: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::EQU));
	}

	case Token::Type::Neq: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::NEQ));
	}

	case Token::Type::Dequal: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::STRING));
	}

	case Token::Type::For: {
		ForNode::ForType fortyp = ForNode::ForType::FILES;
		ParenthesesNode *cond;
		Node *body, *current;
		StringNode *options;
		char id;
		bool loopSubjectDefined = false;

		int _i = i+1, skip = 1;
		for(; command_terminators.count(tokens[_i].type) == 0 && _i < tokens.size()-1; _i+=skip, skip = 1) {
			std::tie(skip, current) = makeNode(tokens, _i, level+1);
			if(current == nullptr) { 
				delete current;
				continue;
			}

			switch(current->type & BARETYPE) {
			case Node::Type::Switch: {
				SwitchNode *sw = (SwitchNode *)(current);
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
			case Node::Type::Id: {
				id = current->str[0];
				break;
			}
			case Node::Type::Parentheses: {
				if(loopSubjectDefined) body = current;
				else {
			case Node::Type::Call:
					cond = current;
				}
				break;
			}
			case Node::Type::String: {
				options = current;
				break;
			}
			default: break;
			}
		}

		return std::make_pair(_i-i, new ForNode(fortyp, id, cond, body, options));
	}

	case Token::Type::If: {
		int _i = i+1, skip = 1;

		CompareNode *cmp;
		Node *sucessRoot = nullptr, *failureRoot = nullptr;
		bool invert = false, caseInsensitive = false;
		bool elseEncountered = false;

		for(; command_terminators.count(tokens[_i].type) == 0 && _i < tokens.size()-1; _i+=skip, skip = 1) {
			if(tokens[i].type == Token::Type::Else && sucessRoot != nullptr) {
				elseEncountered = true;
				continue;
			}
			auto [skp, nod] = makeNode(tokens, _i, level+1);
			if(nod != nullptr) {
				switch(nod->type & BARETYPE) {
				case Node::Type::Compare: {
					cmp = nod;
					break;
				}
				case Node::Type::Switch: {
					SwitchNode *sw = (SwitchNode *)(nod);
					if(strcasecmp(sw->str, "i") == 0) caseInsensitive = true;
					else if(strcasecmp(sw->str, "not") == 0) invert = true;
					break;
				}
				// case Node::Type::Id: {
				// 	// IdNode *id = (IdNode *)(nod);
				// 	// if(strcasecmp(id->str, "defined") == 0) 
				// 	// else if(strcasecmp(id->str, "exist") == 0)
				// 	break;
				// }
				case Node::Type::Parentheses:
				case Node::Type::Call: {
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

	case Token::Type::String: return std::make_pair(1, new StringNode((char*)tokens[i].value));
	case Token::Type::Switch: return std::make_pair(1, new SwitchNode((char*)tokens[i].value, tokens[i].additionalData));
	case Token::Type::Label: return std::make_pair(1, new LabelNode((char*)tokens[i].value, i-1));

	case Token::Type::Id:
		if(level != 0) { return std::make_pair(1, new IdNode((char*)tokens[i].value)); break; }
		// fallthrough
	case Token::Type::BuiltIn: {
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

		if(tokens.size() >= i-1) return std::make_pair(_i-i, (Node*)new CallNode((char*)tokens[i].value, args, tokens[i-1].type == Token::Type::At));
		else return std::make_pair(_i-i, (Node*)new CallNode((char*)tokens[i].value, args));
	}

	default: return std::make_pair(1, nullptr);
	}
}

Parser::Parser(char *buffer, size_t length)
 : lexer(buffer, length)
{
	this->tokens = lexer.lexBuffer();
}

#include <magic_enum.hpp>

void Parser::parse() {
	int i, skip;

	SortTokens(tokens, 0, 0);

	printf("[%s] === Token printout ===\n", __FILE__);
	for(int x = 0; x < tokens.size()-1; x++) {
		printf("[%s] %d\t%s\n", __FILE__, tokens[x].type, magic_enum::enum_name((Token::Type)(tokens[x].type)).data());
	}

	for(i = 0, skip = 1; i < tokens.size()-1; i+=skip, skip = 1) {
		auto [_skip, _node] = makeNode(tokens, i, 0);
		if(_node == nullptr) continue;
		this->nodes.push_back(_node);
		skip = _skip;
	}
}
