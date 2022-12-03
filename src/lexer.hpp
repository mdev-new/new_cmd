#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

enum Tokens {
	TOK_UNDEFINED,
	TOK_SPACE,
	TOK_NUMBER,
	TOK_OPENING_BRACKET,
	TOK_CLOSING_BRACKET,
	TOK_PLUS,
	TOK_MINUS,
	TOK_MULTIPLY,
	TOK_DIVIDE,
	TOK_EQUALS,
	TOK_PIPE,
	TOK_AND,
	TOK_PERCENT,
	TOK_LOGICALNOT,
	TOK_TILDE,
	TOK_XOR,
	TOK_COMMA,
	TOK_LT,
	TOK_GT
};

struct Token {
	short token;
	LONG64 value;
};

struct LexedFile {
	Token *tokens;
	int sizeOfTokens;
};

LexedFile lex(HANDLE hFile);