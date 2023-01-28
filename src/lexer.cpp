#include "lexer.hpp"
#include "shared.hh"
#include <cstdlib>
#include <cstdio>
#include <unordered_map>
#include <utility>

#include "commands.hh"

static char *strndup(const char *str, size_t chars) {
	char *buffer = calloc(chars +1, sizeof (char));
	if (!buffer) return nullptr;

	for (int n = 0; ((n < chars) && (str[n] != 0)); n++) buffer[n] = str[n];
	return buffer;
}

Lexer::Lexer(uint8_t *buffer, size_t size)
 : buffer(buffer),
   length(size),
   idx(0)
{
}

Token Lexer::get() {
	// i guess table could also work for simple tokens
#define ReturnToken(x) idx++; return (Token){x, 0, 0, idx-1, 1}

	switch(buffer[idx]) {
	case '(': ReturnToken(Token::Type::Leftparen);
	case ')': ReturnToken(Token::Type::Rightparen);
	case '<': ReturnToken(Token::Type::PipeIn);
	case '>': ReturnToken(Token::Type::PipeOut);
	case '=': ReturnToken(Token::Type::Equals);
	case '+': ReturnToken(Token::Type::Plus);
	case '-': ReturnToken(Token::Type::Minus);
	case '*': ReturnToken(Token::Type::Asterisk);
	case '/': ReturnToken(Token::Type::Slash);
	case '.': ReturnToken(Token::Type::Dot);
	case ',': ReturnToken(Token::Type::Comma);
	case ':': ReturnToken(Token::Type::Colon);
	case ';': ReturnToken(Token::Type::Semicolon);
	case '|': ReturnToken(Token::Type::Pipe);
	case '%': ReturnToken(Token::Type::Percent);
	case '~': ReturnToken(Token::Type::Tilde);
	case '&': ReturnToken(Token::Type::And);
	case '@': ReturnToken(Token::Type::At);
	case '!': ReturnToken(Token::Type::Exclamation);
	case '\'':
	case '"': {
		char x = buffer[idx++];
		size_t orig_idx = idx;
		while(buffer[idx++] != x);
		return (Token){(buffer[orig_idx-1] == '"')? Token::Type::String : Token::Type::SinglequoteString, strndup(&buffer[orig_idx], idx-1-orig_idx), idx-1-orig_idx, orig_idx-1, idx-orig_idx+1};
	}
	case '\n': ReturnToken(Token::Type::Space);
	default: break;
	}

	if(isdigit(buffer[idx])) {
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
	}

	if(validIdStart(buffer[idx])) {
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
