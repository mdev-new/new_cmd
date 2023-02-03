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

#include <patterns/patterns.hpp>
#include <magic_enum.hpp>

#define TCAST(type, val) ((type)val)

// this whole thing probably needs a full rewrite, using something like expect() and consume()...

// stackoverflow saved my ass
template <typename t> void move(std::vector<t>& v, size_t oldIndex, size_t newIndex) {
	if (oldIndex > newIndex)
		std::rotate(v.rend() - oldIndex - 1, v.rend() - oldIndex, v.rend() - newIndex);
	else
		std::rotate(v.begin() + oldIndex, v.begin() + oldIndex + 1, v.begin() + newIndex + 1);
}

// stage 2 lexer basically
int SortTokens(std::vector<Token> &tokens, int start, int breaktok) {
	long i = start, skip = 0, parenStart = 0;

	static auto parens = [&] {
		parenStart = i;
		skip = SortTokens(tokens, i+1, Token::Type::RightParen);
	};

	static auto sortBinOps = [&] {
		move(tokens, i, parenStart);
		parenStart = 0;
	};

	static auto prefix = [&] {
		std::swap(tokens[i-1], tokens[i]);
	};

	static auto sw = [&] {
		char pref = (tokens[i].type == Token::Type::Slash)? '/' : '-';
		tokens.erase(tokens.begin() + i);
		tokens[i] = (Token){Token::Type::Switch, tokens[i].value, pref};
	};

	static auto label = [&] {
		char pref = ':';
		tokens.erase(tokens.begin() + i);
		tokens[i+1] = (Token){Token::Type::Label, tokens[i+1].value, pref};
	};

	for(; breaktok? (tokens[i].type != breaktok && i < tokens.size()-1) : (i < tokens.size()-1); i+=skip, skip=1) {
		match(tokens[i].type, tokens[i+1].type, tokens[i+2].type) (
			pattern(Token::Type::LeftParen, _, _) = parens,

			pattern(Token::Type::RightParen, anyof(
				Token::Type::Plus,
				Token::Type::Minus,
				Token::Type::Asterisk,
				Token::Type::Slash,
				Token::Type::And,
				Token::Type::Caret,
				Token::Type::Pipe,
				Token::Type::Caret
			), _) = sortBinOps,

			pattern(Token::Type::Number, anyof(
				Token::Type::Plus,
				Token::Type::Minus,
				Token::Type::Asterisk,
				Token::Type::Slash,
				Token::Type::And,
				Token::Type::Caret,
				Token::Type::Pipe,
				Token::Type::Caret
			), _) = prefix,

			pattern(Token::Type::Number, anyof(
				Token::Type::PlusEquals,
				Token::Type::MinusEquals,
				Token::Type::TimesEquals,
				Token::Type::DivideEquals,
				Token::Type::AndEquals,
				Token::Type::OrEquals,
				Token::Type::XorEquals
			), _) = prefix,

			pattern(anyof(
				Token::Type::Gtr,
				Token::Type::Lss,
				Token::Type::Geq,
				Token::Type::Leq,
				Token::Type::Equ,
				Token::Type::Neq
			), _, _) = prefix,

			// todo reduce multiple consecutive slashes
			// todo move rhs to the front (incase thi happens =xyz0, =0xyz would be easier to parse)
			pattern(anyof(Token::Type::Equals, Token::Type::Dequal), _, _) = prefix,

			pattern(anyof(Token::Type::Slash, Token::Type::Minus), Token::Type::Id, _) = sw,

			pattern(Token::Type::Colon, Token::Type::Id, _) = label,
			pattern(_, _, _) = [&] { return; }
		);

		// todo support %%x and %%~x variables
		// also support variable replacements and substitutions
		// if(current.type == Token::Type::Percent && (tokens[i+1].type == Token::Type::Id || tokens[i+1].type == Token::Type::BuiltIn)) {
		// 	tokens.erase(tokens.begin() + i);
		// 	tokens.erase(tokens.begin() + i + 2);
		// 	tokens[i] = current = (Token){Token::Type::EnvVar, tokens[i+1].value, false};
		// 	continue;
		// }
	}

	return i;
}

static std::unordered_set<decltype(Token::type)> command_terminators = {
	Token::Type::Pipe,
	Token::Type::LogicalAnd,
	Token::Type::And,
	Token::Type::LogicalOr,
	Token::Type::Pipe,
	Token::Type::PipeIn,
	Token::Type::PipeOut,
	Token::Type::PipeOutAppend,
	Token::Type::Space,
	Token::Type::Eof
};

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

	case Token::Type::LeftParen: {
		int j = i+1, skip = 1;
		ParenthesesNode *n = new ParenthesesNode();
		Node *current = nullptr;
		for(; tokens[j].type != Token::Type::RightParen && tokens[j].type != Token::Type::Eof; j+=skip, skip=1) {
			std::tie(skip, current) = makeNode(tokens, j, 0);
			if(current == nullptr) continue;
			n->children->push_back(current);
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
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::Less));
	}

	case Token::Type::Leq: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::LessOrEqual));
	}

	case Token::Type::Gtr: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::Greater));
	}

	case Token::Type::Geq: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::GreaterOrEqual));
	}

	case Token::Type::Equ: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::Equal));
	}

	case Token::Type::Neq: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::NotEqual));
	}

	case Token::Type::Dequal: {
		auto [lskip, lhs] = makeNode(tokens, i+1, level+1);
		auto [rskip, rhs] = makeNode(tokens, i+2, level+1);
		return std::make_pair(lskip+rskip+1, new CompareNode(lhs, rhs, CompareNode::CompareType::String));
	}

	case Token::Type::For: {
		ForNode::ForType fortyp = ForNode::ForType::Files;
		ParenthesesNode *cond = nullptr;
		Node *body = nullptr, *current;
		StringNode *options;
		char id = 0;

		int _i = i+1, skip = 1;
		for(; command_terminators.count(tokens[_i].type) == 0 && _i < tokens.size()-1; _i+=skip, skip = 1) {
			std::tie(skip, current) = makeNode(tokens, _i, (cond && id) ? 0 : level+1);
			if(current == nullptr) continue;

			switch(current->type & BARETYPE) {
			case Node::Type::Switch: {
				SwitchNode *sw = (SwitchNode *)(current);
				switch(sw->str[0] | 0x20) {
				case 'l': {
					fortyp = ForNode::ForType::Numbers;
					break;
				}
				case 'f': {
					break;
				}
				case 'd': {
					fortyp = ForNode::ForType::Folders;
					break;
				}
				case 'r': {
					fortyp = ForNode::ForType::FilesRooted;
					break;
				}
				default: break;
				}
				break;
			}
			case Node::Type::Id: {
				id = current->str[0];
				break;
			}
			case Node::Type::Parentheses: {
				if(!cond) {
					cond = current;
				}
				else {
			case Node::Type::Call:
					body = current;
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

		return std::make_pair(_i-i+1, new ForNode(fortyp, id, cond, body, options));
	}

	case Token::Type::If: {
		int _i = i+1, skip = 1;

		CompareNode *cmp = nullptr;
		Node *sucessRoot = nullptr, *failureRoot = nullptr, *current = nullptr;
		bool invert = false, caseInsensitive = false;
		bool elseEncountered = false;

		for(; command_terminators.count(tokens[_i].type) == 0 && _i < tokens.size()-1; _i+=skip, skip = 1) {
			if(tokens[i].type == Token::Type::Else && sucessRoot != nullptr) {
				elseEncountered = true;
				continue;
			}
			std::tie(skip, current) = makeNode(tokens, _i, (!cmp)? level+1 : 0);
			if(current == nullptr) continue;

			switch(current->type & BARETYPE) {
			case Node::Type::Compare: {
				cmp = current;
				break;
			}
			case Node::Type::Switch: {
				SwitchNode *sw = (SwitchNode *)(current);
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
			case Node::Type::If:
			case Node::Type::For:
			case Node::Type::Parentheses:
			case Node::Type::Call: {
				//printf("%s\n", nod->stringify().first);
				if(elseEncountered) failureRoot = current;
				else sucessRoot = current;
				break;
			}
			default: break;
			}
		}
	
		return std::make_pair(_i-i, new IfNode(cmp, sucessRoot, failureRoot, invert, caseInsensitive));
	}

	case Token::Type::String: return std::make_pair(1, new StringNode((char*)tokens[i].value));
	case Token::Type::Switch: return std::make_pair(1, new SwitchNode((char*)tokens[i].value, tokens[i].additionalData));
	case Token::Type::Label: return std::make_pair(1, new LabelNode((char*)tokens[i].value, i-1));

	//case Token::Type::EnvVar: return std::make_pair(1, new EnvVarNode((char*)tokens[i].value, tokens[i].additionalData));

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
 : lexer(buffer, length),
 idx(0)
{
	this->tokens = lexer.lexBuffer();
}

void Parser::parse() {
	int i, skip;

	SortTokens(tokens, 0, 0);

	fprintf(stderr, "[%s] === Token printout ===\n", __FILE__);
	for(int x = 0; x < tokens.size()-1; x++) {
		fprintf(stderr, "[%s] %d\t%s\n", __FILE__, tokens[x].type, magic_enum::enum_name((Token::Type)(tokens[x].type)).data());
	}

	for(i = 0, skip = 1; i < tokens.size()-1; i+=skip, skip = 1) {
		auto [_skip, _node] = makeNode(tokens, i, 0);
		if(_node == nullptr) continue;
		this->nodes.push_back(_node);
		skip = _skip;
	}
}
