#pragma once
#include <cstdint>
#include <cstdint>
#include <cstddef>
#include <utility>
#include <list>
#include <vector>

enum TokenType {
	TOK_INVAL = 0,
	TOK_EOF = 1,
	TOK_SPACE,
	TOK_ID,
	TOK_NUMBER,
	TOK_STRING,
	TOK_SWITCH,

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
	TOK_AND,

	TOK_BUILTIN,
	TOK_FOR,
	TOK_DO,
	TOK_IF,
	TOK_ELSE,
	TOK_IN,
};

struct Token {
	short type;
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
	std::vector<Token> lexBuffer();

	bool eof() { return length == 0 || idx >= length || buffer[idx] == 0; }
	bool isdigit(char c) { return '0' <= c && c <= '9'; }
	bool isalpha(char c) { return 'a' <= c && c <= 'z' || 'A' <= c && c <= 'Z'; }
	bool validIdStart(char c) { return (isalpha(c) || c == '_'); }
	bool validIdBody(char c) { return validIdStart(c) || isdigit(c); }
};
