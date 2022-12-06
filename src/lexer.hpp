#pragma once
//#define WIN32_LEAN_AND_MEAN
//#include <Windows.h>

#include <cstdint>
#include <unordered_map>

std::unordered_map<char *, int> multicharMapping = {
	"for": TOK_FOR,
};

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

///
	TOK_FOR,
	TOK_DO,
	TOK_SET,
	TOK_MKDIR,
	TOK_DEL,
	TOK_ECHO,
	TOK_CD,
	TOK_DIR
};

constexpr uint64_t consthash(const char *text) {
  uint64_t h = 525201411107845655ull;
  for (int i = 0;text[i];++i) {
    h ^= text[i];
    h *= 0x5bd1e9955bd1e995;
    h ^= h >> 47;
  }
  return h;
}

struct Token {
	short token;
	uint64_t value;
};

struct LexedFile {
	Token *tokens;
	int sizeOfTokens;
};

LexedFile lex(char *buffer, int fileSize);
