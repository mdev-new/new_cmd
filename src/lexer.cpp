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
	case '(': ReturnToken(TokenType::Leftparen);
	case ')': ReturnToken(TokenType::Rightparen);
	case '<': ReturnToken(TokenType::PipeIn);
	case '>': ReturnToken(TokenType::PipeOut);
	case '=': ReturnToken(TokenType::Equals);
	case '+': ReturnToken(TokenType::Plus);
	case '-': ReturnToken(TokenType::Minus);
	case '*': ReturnToken(TokenType::Asterisk);
	case '/': ReturnToken(TokenType::Slash);
	case '.': ReturnToken(TokenType::Dot);
	case ',': ReturnToken(TokenType::Comma);
	case ':': ReturnToken(TokenType::Colon);
	case ';': ReturnToken(TokenType::Semicolon);
	case '|': ReturnToken(TokenType::Pipe);
	case '%': ReturnToken(TokenType::Percent);
	case '~': ReturnToken(TokenType::Tilde);
	case '&': ReturnToken(TokenType::And);
	case '@': ReturnToken(TokenType::At);
	case '!': ReturnToken(TokenType::Exclamation);
	case '\'':
	case '"': {
		char x = buffer[idx++];
		size_t orig_idx = idx;
		while(buffer[idx++] != x);
		return (Token){(buffer[orig_idx-1] == '"')? TokenType::String : TokenType::SinglequoteString, strndup(&buffer[orig_idx], idx-1-orig_idx), idx-1-orig_idx, orig_idx-1, idx-orig_idx+1};
	}
	case '\n': ReturnToken(TokenType::Space);
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
		return (Token){TokenType::Number, num, 0, orig_idx, idx-orig_idx};
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
		} else return (Token){TokenType::Id, identifier, len, idstart, len};
	}

	ReturnToken(TokenType::Inval);
}

std::vector<Token> Lexer::lexBuffer() {
	std::vector<Token> list;

	while(!eof()) {
		auto token = get();
		if(token.type != TokenType::Inval) list.push_back(token);
	}

	list.push_back((Token){TokenType::Eof, 0, 0, 0, 0});
	return list;
}
