#include "lexer.hpp"
#include <cstdlib>
#include <cstdio>
#include <unordered_map>
#include <utility>
#include "interpreter.hh"

Lexer::Lexer(uint8_t *buffer, size_t size)
 : buffer(buffer),
   length(size),
   idx(0)
{
}

Token Lexer::get() {
	// i guess table could also work for simple tokens
	switch(buffer[idx]) {
	case '(': idx++; return (Token){TOK_LEFTPAREN, 0, 0};
	case ')': idx++; return (Token){TOK_RIGHTPAREN, 0, 0};
	case '<': idx++; return (Token){TOK_LT, 0, 0};
	case '>': idx++; return (Token){TOK_GT, 0, 0};
	case '=': idx++; return (Token){TOK_EQUALS, 0, 0};
	case '+': idx++; return (Token){TOK_PLUS, 0, 0};
	case '-': idx++; return (Token){TOK_MINUS, 0, 0};
	case '*': idx++; return (Token){TOK_ASTERISK, 0, 0};
	case '/': idx++; return (Token){TOK_SLASH, 0, 0};
	case '.': idx++; return (Token){TOK_DOT, 0, 0};
	case ',': idx++; return (Token){TOK_COMMA, 0, 0};
	case ':': idx++; return (Token){TOK_COLON, 0, 0};
	case ';': idx++; return (Token){TOK_SEMICOLON, 0, 0};
	case '|': idx++; return (Token){TOK_PIPE, 0, 0};
	case '%': idx++; return (Token){TOK_PERCENT, 0, 0};
	case '~': idx++; return (Token){TOK_TILDE, 0, 0};
	case '\'':
	case '"': {
		char x = buffer[idx++];
		size_t orig_idx = idx;
		while(buffer[idx++] != x);
		return (Token){TOK_STRING, strndup(&buffer[orig_idx], idx-1-orig_idx), idx-1-orig_idx};
	}
	default: break;
	}

	if(isdigit(buffer[idx])) {
		int num = 0;
		while(isdigit(buffer[idx])) {
			num = num * 10 + buffer[idx] - '0';
			idx++;
		}
		return (Token){TOK_NUMBER, num, 0};
	}

	if(validIdStart(buffer[idx])) {
		size_t idstart = idx;
		idx++;
		while(validIdBody(buffer[idx])) idx++;

		char *identifier = strndup(buffer+idstart, idx - idstart);

		if(multicharMapping.count(hash(identifier)) > 0) {
			auto [_token, _value] = multicharMapping.at(hash(identifier));
			free(identifier);
			return (Token){_token, _value, 0};
		} else return (Token){TOK_ID, identifier, idx - idstart};
	}

	idx++;
	return (Token){TOK_INVAL, -1, -1};
}

std::pair<size_t, Token *> Lexer::lexBuffer() {
	int created = 0, treshold = 128;
	Token *tokenbuf = malloc(treshold*sizeof(Token));

	int i = 0;

	while(!eof()) {
		if(created >= treshold) tokenbuf = realloc(tokenbuf, (treshold += 128)*sizeof(Token));
		auto token = get();
		if(token.type != TOK_INVAL) tokenbuf[created++] = token;
	}

	tokenbuf = realloc(tokenbuf, created*sizeof(Token));
	return std::make_pair(created, tokenbuf);
}