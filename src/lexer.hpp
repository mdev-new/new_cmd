#pragma once
//#define WIN32_LEAN_AND_MEAN
//#include <Windows.h>

#include <cstdint>
//#include <unordered_map>

enum TokenType {
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

	TOK_SWITCH,

	TOK_ENVVAR,
	TOK_BUILTIN, // builtins that do not need to be tokens

	TOK_FOR,
	TOK_DO,
	TOK_IF,
	TOK_ELSE,
};

struct Token {
	short token;
	uint64_t value;
	uint64_t additionalData;
};

struct LexedFile {
	// i could use std::vector here but whatever
	Token *tokens;
	int noOfTokens;
};

LexedFile lex(char *buffer, int fileSize);
