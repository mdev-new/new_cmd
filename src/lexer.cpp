#include "lexer.hpp"
#include "shared.hh"
#include <cstdlib>
#include <cstdio>
#include <unordered_map>
#include <utility>

#include "commands.hh"

#include <patterns/patterns.hpp>

// todo regex

static char *strndup(const char *str, size_t chars) {
	char *buffer = calloc(chars +1, sizeof (char));
	if (!buffer) return nullptr;

	for (int n = 0; ((n < chars) && (str[n] != 0)); n++) buffer[n] = str[n];
	return buffer;
}

decltype(multicharMapping) multicharTokens = {
	fe("do", Token::Type::Do, nullptr)
	fe("else", Token::Type::Else, nullptr)
	fe("in", Token::Type::In, nullptr)
	fe("for", Token::Type::For, nullptr)
	fe("if", Token::Type::If, nullptr)

	fe("gtr", Token::Type::Gtr, nullptr)
	fe("geq", Token::Type::Geq, nullptr)
	fe("lss", Token::Type::Lss, nullptr)
	fe("leq", Token::Type::Leq, nullptr)
	fe("equ", Token::Type::Equ, nullptr)
	fe("neq", Token::Type::Neq, nullptr)
};

Lexer::Lexer(uint8_t *buffer, size_t size)
 : buffer(buffer),
   length(size),
   idx(0)
{
	for (auto &it : multicharTokens) {
		multicharMapping.insert(it);
	}
}

Token Lexer::get() {
	// i guess table could also work for simple tokens
#define ReturnToken(x) idx++; return (Token){x, 0, 0, idx-1, 1}
#define ReturnToken1(x, y) idx+=y; return (Token){x, 0, 0, idx-y, 1}

	static auto processQuotes = [&] {
		char x = buffer[idx++];
		size_t orig_idx = idx;
		while(buffer[idx++] != x);
		return (Token){
			(buffer[orig_idx-1] == '"')? Token::Type::String : Token::Type::SingleQuoteString,
			strndup(&buffer[orig_idx], idx-1-orig_idx),
			idx-1-orig_idx,
			orig_idx-1,
			idx-orig_idx+1
		};
	};

	static auto processDigit = [&] {
		int num = 0;
		size_t orig_idx = idx;
		while(isdigit(buffer[idx]) || isxdigit(buffer[idx])) {
			if(isxdigit(buffer[idx]))
				num = num * 10 + (10 + buffer[idx] - (islxdigit(buffer[idx])? 'a' : 'A'));
			else
				num = num * 10 + buffer[idx] - '0';
			idx++;
		}
		return (Token){Token::Type::Number, num, 0, orig_idx, idx-orig_idx};
	};

	static auto processId = [&] {
		size_t idstart = idx;
		idx++;
		while(validIdBody(buffer[idx])) idx++;

		int len = idx - idstart;
		char *identifier = strndup(buffer+idstart, len);

		_hashtype_ h = _hashfunc_(identifier, true);
		if(multicharMapping.count(h) > 0) {
			auto _token = multicharMapping.at(h).first;
			return (Token){_token, identifier, 0, idstart, len};
		} else return (Token){Token::Type::Id, identifier, len, idstart, len};
	};

	Token t = match(buffer[idx], buffer[idx+1], buffer[idx+2]) (
		pattern('=', '=', _) = [&] { ReturnToken1(Token::Type::Dequal, 2); },
		pattern('>', '>', _) = [&] { ReturnToken(Token::Type::PipeOutAppend); },
		pattern('|', '|', _) = [&] { ReturnToken1(Token::Type::LogicalOr, 2); },
		pattern('&', '&', _) = [&] { ReturnToken1(Token::Type::LogicalAnd, 2); },

		pattern('+', '=', _) = [&] { ReturnToken1(Token::Type::PlusEquals, 2); },
		pattern('-', '=', _) = [&] { ReturnToken1(Token::Type::MinusEquals, 2); },
		pattern('*', '=', _) = [&] { ReturnToken1(Token::Type::TimesEquals, 2); },
		pattern('/', '=', _) = [&] { ReturnToken1(Token::Type::DivideEquals, 2); },
		pattern('|', '=', _) = [&] { ReturnToken1(Token::Type::OrEquals, 2); },
		pattern('&', '=', _) = [&] { ReturnToken1(Token::Type::AndEquals, 2); },
		pattern('^', '=', _) = [&] { ReturnToken1(Token::Type::XorEquals, 2); },

		pattern('(', _, _) = [&] { ReturnToken(Token::Type::LeftParen); },
		pattern(')', _, _) = [&] { ReturnToken(Token::Type::RightParen); },
		pattern('<', _, _) = [&] { ReturnToken(Token::Type::PipeIn); },
		pattern('>', _, _) = [&] { ReturnToken(Token::Type::PipeOut); },
		pattern('=', _, _) = [&] { ReturnToken(Token::Type::Equals); },
		pattern('+', _, _) = [&] { ReturnToken(Token::Type::Plus); },
		pattern('-', _, _) = [&] { ReturnToken(Token::Type::Minus); },
		pattern('*', _, _) = [&] { ReturnToken(Token::Type::Asterisk); },
		pattern('/', _, _) = [&] { ReturnToken(Token::Type::Slash); },
		pattern('.', _, _) = [&] { ReturnToken(Token::Type::Dot); },
		pattern(',', _, _) = [&] { ReturnToken(Token::Type::Comma); },
		pattern(':', _, _) = [&] { ReturnToken(Token::Type::Colon); },
		pattern(';', _, _) = [&] { ReturnToken(Token::Type::Semicolon); },
		pattern('|', _, _) = [&] { ReturnToken(Token::Type::Pipe); },
		pattern('%', _, _) = [&] { ReturnToken(Token::Type::Percent); },
		pattern('~', _, _) = [&] { ReturnToken(Token::Type::Tilde); },
		pattern('&', _, _) = [&] { ReturnToken(Token::Type::And); },
		pattern('@', _, _) = [&] { ReturnToken(Token::Type::At); },
		pattern('!', _, _) = [&] { ReturnToken(Token::Type::Exclamation); },
		pattern('\n', _, _) = [&] { ReturnToken(Token::Type::Space); },
		pattern('\'', _, _) = processQuotes,
		pattern('"', _, _) = processQuotes,
		pattern(isdigit(x), _, _) = processDigit,
		pattern(validIdStart(_), _, _) = processId,
		pattern(_, _, _) = [&] { return (Token){Token::Type::Inval, 0, 0, 0, 0}; }
	);

	if(t.type != Token::Type::Inval) {
		return t;
	}

	ReturnToken(Token::Type::Inval);
}

std::vector<Token> Lexer::lexBuffer() {
	std::vector<Token> list;

	while(!eof()) {
		auto token = get();
		if(token.type != Token::Type::Inval) list.push_back(token);
	}

	list.push_back((Token){Token::Type::Eof, 0, 0, 0, 0});
	return list;
}
