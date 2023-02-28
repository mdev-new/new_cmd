#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>

#include "token.hpp"

class Lexer {
	uint8_t *buffer;
	size_t length;
	size_t idx = 0;

public:
	Lexer(uint8_t *buffer, size_t size);
	Token get();
	std::vector<Token> lexBuffer();

private:
	Token processPossibleEnvVar(decltype(idx) idx, int fallback);

	bool eof() { return length == 0 || idx >= length || buffer[idx] == 0; }
	const bool isdigit(char c) const { return '0' <= c && c <= '9'; }
	const bool isalpha(char c) const { return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'); }
	const bool validIdStart(char c) const { return (isalpha(c) || c == '_'); }
	const bool validIdBody(char c) const { return validIdStart(c) || isdigit(c); }
	const bool islxdigit(char c) const { return c >= 'a' && c <= 'f'; }
	const bool isuxdigit(char c) const { return c >= 'A' && c <= 'F'; }
	const bool isxdigit(char c) const { return islxdigit(c) || isuxdigit(c); }
};
