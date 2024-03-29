#include "lexer.hpp"
#include <cstdlib>
#include <cstdio>
#include <unordered_map>
#include <utility>

#include "interpreter/commands.hh"

#include <mpark/patterns.hpp>
#include <regex>
#include <string>

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
	/// todo better way of doing this
	// this doesnt support multiple instances of lexer
	for (auto &it : multicharTokens) {
		multicharMapping.insert(it);
	}
}

// LIMITATION: env vars cannot start with numbers
//todo nested env vars

// this is fucking ugly but who cares
Token Lexer::processPossibleEnvVar(size_t idx, int fallbackToken) {
  char *m1, *m2 = nullptr;
  char *newlPtr = nullptr;
  char *line = strndup(this->buffer, (newlPtr = strchr(this->buffer, '\n')-(char*)(this->buffer)));
  char *envVarName = nullptr;

  // todo check for commas (set /a x=1%2,y=4%2)

  if(m1 = strchr(newlPtr, '%')+1) {
    if(m2 = strchr(newlPtr, '%')-1) {
      envVarName = strndup(m1, m2-m1);
    }
  }
  else if(m1 = strchr(newlPtr, '!')+1) {
    if(m2 = strchr(newlPtr, '!')-1) {
      envVarName = strndup(m1, m2-m1);
    }
  }
  else return (Token){fallbackToken, 0, 0, this->idx++, 1};

//  if(matching_percents_found | matching_exclamations_found) {
//    while(next_percent != nullptr || next_exclamation != nullptr) {
//        find_second_percent | find_second_exclamation;
//        replace content between them with %s and store original content in strduped var
//    }
//  }
}

Token Lexer::get() {
	// i guess table could also work for simple tokens
#define ReturnToken(x) return (Token){x, 0, 0, idx++, 1}
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

	// this is ugly but works (acessing the buffer again)
	static auto processEnvVarOrMathSym = [&] {
		return processPossibleEnvVar(idx, buffer[idx] == '!'? Token::Type::Exclamation : Token::Type::Percent);
	};

	using namespace mpark::patterns;
	Token t = match(buffer[idx], buffer[idx+1], isdigit(buffer[idx]), isalpha(buffer[idx])) (
		pattern('=', '=', _, _) = [&] { ReturnToken1(Token::Type::Dequal, 2); },
		pattern('>', '>', _, _) = [&] { ReturnToken1(Token::Type::PipeOutAppend, 2); },
		pattern('|', '|', _, _) = [&] { ReturnToken1(Token::Type::LogicalOr, 2); },
		pattern('&', '&', _, _) = [&] { ReturnToken1(Token::Type::LogicalAnd, 2); },

		pattern('+', '=', _, _) = [&] { ReturnToken1(Token::Type::PlusEquals, 2); },
		pattern('-', '=', _, _) = [&] { ReturnToken1(Token::Type::MinusEquals, 2); },
		pattern('*', '=', _, _) = [&] { ReturnToken1(Token::Type::TimesEquals, 2); },
		pattern('/', '=', _, _) = [&] { ReturnToken1(Token::Type::DivideEquals, 2); },
		pattern('|', '=', _, _) = [&] { ReturnToken1(Token::Type::OrEquals, 2); },
		pattern('&', '=', _, _) = [&] { ReturnToken1(Token::Type::AndEquals, 2); },
		pattern('^', '=', _, _) = [&] { ReturnToken1(Token::Type::XorEquals, 2); },

		pattern('%', '%', _, _) = [&] { idx+=2; return processId(); },
		pattern('%', _, _, _) = processEnvVarOrMathSym,
		pattern('!', _, _, _) = processEnvVarOrMathSym,

		pattern('(', _, _, _) = [&] { ReturnToken(Token::Type::LeftParen); },
		pattern(')', _, _, _) = [&] { ReturnToken(Token::Type::RightParen); },
		pattern('<', _, _, _) = [&] { ReturnToken(Token::Type::PipeIn); },
		pattern('>', _, _, _) = [&] { ReturnToken(Token::Type::PipeOut); },
		pattern('=', _, _, _) = [&] { ReturnToken(Token::Type::Equals); },
		pattern('+', _, _, _) = [&] { ReturnToken(Token::Type::Plus); },
		pattern('-', _, _, _) = [&] { ReturnToken(Token::Type::Minus); },
		pattern('*', _, _, _) = [&] { ReturnToken(Token::Type::Asterisk); },
		pattern('/', _, _, _) = [&] { ReturnToken(Token::Type::Slash); },
		pattern(',', _, _, _) = [&] { ReturnToken(Token::Type::Comma); },
		//pattern(':', _, _, _) = [&] { ReturnToken(Token::Type::Colon); },
		pattern('|', _, _, _) = [&] { ReturnToken(Token::Type::Pipe); },
		pattern('~', _, _, _) = [&] { ReturnToken(Token::Type::Tilde); },
		pattern('&', _, _, _) = [&] { ReturnToken(Token::Type::And); },
		pattern('@', _, _, _) = [&] { ReturnToken(Token::Type::At); },
		pattern('\n', _, _, _) = [&] { ReturnToken(Token::Type::Space); },
		pattern('\'', _, _, _) = processQuotes,
		pattern('"', _, _, _) = processQuotes,
		pattern(_, _, true, _) = processDigit,
		pattern(_, _, _, true) = processId,
		pattern(_, _, _, _) = [&] { return (Token){Token::Type::Inval, 0, 0, 0, 0}; }
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
