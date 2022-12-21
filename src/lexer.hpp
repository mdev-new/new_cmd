#pragma once
#include <cstdint>
#include <cstdint>
#include <cstddef>
#include <utility>

enum TokenType {
	TOK_INVAL = -1,
	TOK_EOF = 0,
	TOK_SPACE,
	TOK_ID,
	TOK_NUMBER,
	TOK_STRING,

	TOK_LEFTPAREN,
	TOK_RIGHTPAREN,
	TOK_LT,
	TOK_GT,
	TOK_EQUALS,
	TOK_PLUS,
	TOK_MINUS,
	TOK_ASTERISK,
	TOK_SLASH,
	TOK_DOT,
	TOK_COMMA,
	TOK_COLON,
	TOK_SEMICOLON,
	TOK_SINGLEQUOTE,
	TOK_DOUBLEQUOTE,
	TOK_PIPE,
	TOK_PERCENT,
	TOK_TILDE,

	TOK_BUILTIN,
	TOK_FOR,
	TOK_DO,
	TOK_IF,
	TOK_ELSE,
};

struct Token {
	TokenType type;
	uint64_t value;
	uint64_t additionalData;
};

class Lexer {
	uint8_t *buffer;
	size_t length;

	size_t idx = 0;

public:
	Lexer(uint8_t *buffer, size_t size);
	Token get();
	std::pair<size_t, Token *> lexBuffer();

	bool eof() { return idx >= length || buffer[idx] == 0; }
	bool isnum(char c) {
		return (c >= '0' && c <= '9');
	}

	bool validIdStart(char c) {
		return ((buffer[idx] >= 'A' && buffer[idx] <= 'Z') || (buffer[idx] >= 'a' && buffer[idx] <= 'z') || buffer[idx] == '_');
	}
	
	bool validIdBody(char c) {
		return validIdStart(buffer[idx]) || isnum(buffer[idx]);
	}
};
