#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>

enum TokenType {
	TOK_INVAL = -1,
	TOK_EOF = 0,
	TOK_SPACE,
	TOK_ID,
	TOK_NUMBER,
	TOK_STRING,
	TOK_SINGLEQUOTESTRING,

	// created artificially by parser
	TOK_SWITCH,
	TOK_LABEL,
	TOK_DEQUAL,
	// ---

	TOK_LEFTPAREN,
	TOK_RIGHTPAREN,
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
	TOK_AT,

	TOK_PIPEIN,
	TOK_PIPEOUT,

	TOK_LSS, // LSS
	TOK_GTR, // GTR
	TOK_GEQ,
	TOK_LEQ,
	TOK_EQU,
	TOK_NEQ,

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

	int txtStart;
	int txtLength;
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
	bool isalpha(char c) { return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'); }
	bool validIdStart(char c) { return (isalpha(c) || c == '_'); }
	bool validIdBody(char c) { return validIdStart(c) || isdigit(c); }
	bool islxdigit(char c) { return c >= 'a' && c <= 'f'; }
	bool isuxdigit(char c) { return c >= 'A' && c <= 'F'; }
	bool isxdigit(char c) { return islxdigit(c) || isuxdigit(c); }
};
