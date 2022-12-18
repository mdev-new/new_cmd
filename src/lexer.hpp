#pragma once
//#define WIN32_LEAN_AND_MEAN
//#include <Windows.h>

#include <cstdint>
//#include <unordered_map>

enum TokenType {
	TOK_UNDEFINED,
	TOK_EOF,
	TOK_NUMBER,
	TOK_SWITCH,

	TOK_ENVVAR,
	TOK_BUILTIN, // builtins that do not need to be tokens

	TOK_FOR,
	TOK_DO,
	TOK_IF,
	TOK_ELSE,
    
	TOK_WS_SEPARATOR, // 10
	TOK_OPENING_PAREN,
	TOK_CLOSING_PAREN,
	TOK_PLUS,
	TOK_MINUS,
	TOK_MULTIPLY, // 15
	TOK_DIVIDE,
	TOK_AND,
	TOK_EQUALS,
	TOK_PIPE,
	TOK_PERCENT,
	TOK_LOGICALNOT,
	TOK_TILDE,
	TOK_XOR,
	TOK_COMMA,
	TOK_LT,
	TOK_GT,
	TOK_COLON,
	TOK_QUOTE,
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
