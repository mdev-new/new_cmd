#pragma once
//#define WIN32_LEAN_AND_MEAN
//#include <Windows.h>

#include <cstdint>
//#include <unordered_map>

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
	TOK_GT,
	TOK_COLON,
	TOK_QUOTE,

///
	TOK_FOR,
	TOK_DO,
	TOK_SET,
	TOK_MKDIR,
	TOK_DEL,
	TOK_ECHO,
	TOK_CD,
	TOK_DIR,
	TOK_IF,
	TOK_ELSE,

};

struct Token {
	short token;
	uint64_t value;
	uint64_t additionalData;
};

struct LexedFile {
	Token *tokens;
	int sizeOfTokens;
};

LexedFile lex(char *buffer, int fileSize);
