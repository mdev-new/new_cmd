#include "lexer.hpp"
#include "shared.hh"
#include <cstdlib>
#include <cstdio>
#include <unordered_map>
#include <utility>

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
	case '(': ReturnToken(TOK_LEFTPAREN);
	case ')': ReturnToken(TOK_RIGHTPAREN);
	case '<': ReturnToken(TOK_PIPEIN);
	case '>': ReturnToken(TOK_PIPEOUT);
	case '=': ReturnToken(TOK_EQUALS);
	case '+': ReturnToken(TOK_PLUS);
	case '-': ReturnToken(TOK_MINUS);
	case '*': ReturnToken(TOK_ASTERISK);
	case '/': ReturnToken(TOK_SLASH);
	case '.': ReturnToken(TOK_DOT);
	case ',': ReturnToken(TOK_COMMA);
	case ':': ReturnToken(TOK_COLON);
	case ';': ReturnToken(TOK_SEMICOLON);
	case '|': ReturnToken(TOK_PIPE);
	case '%': ReturnToken(TOK_PERCENT);
	case '~': ReturnToken(TOK_TILDE);
	case '&': ReturnToken(TOK_AND);
	case '@': ReturnToken(TOK_AT);
	case '\'':
	case '"': {
		char x = buffer[idx++];
		size_t orig_idx = idx;
		while(buffer[idx++] != x);
		return (Token){(buffer[orig_idx-1] == '"')? TOK_STRING : TOK_SINGLEQUOTESTRING, strndup(&buffer[orig_idx], idx-1-orig_idx), idx-1-orig_idx, orig_idx-1, idx-orig_idx+1};
	}
	case '\n': ReturnToken(TOK_SPACE);
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
		return (Token){TOK_NUMBER, num, 0, orig_idx, idx-orig_idx};
	}

	if(validIdStart(buffer[idx])) {
		size_t idstart = idx;
		idx++;
		while(validIdBody(buffer[idx])) idx++;

		int len = idx - idstart;
		char *identifier = strndup(buffer+idstart, len);

		_hashtype_ h = _hashfunc_(identifier);
		if(multicharMapping.count(h) > 0) {
			auto _token = multicharMapping.at(h).first;
			return (Token){_token, identifier, 0, idstart, len};
		} else return (Token){TOK_ID, identifier, len, idstart, len};
	}

	ReturnToken(TOK_INVAL);
}

std::vector<Token> Lexer::lexBuffer() {
	std::vector<Token> list;

	while(!eof()) {
		auto token = get();
		if(token.type != TOK_INVAL) list.push_back(token);
	}

	list.push_back((Token){TOK_EOF, 0, 0, 0, 0});
	return list;
}
