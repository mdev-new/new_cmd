#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>

struct Token {
	enum Type {
		Inval = -1,
		Eof = 0,
		Space,
		Id,
		Number,
		String,
		SingleQuoteString = 5,

		// created artificially by parser
		Switch = 6,
		Label,
		// ---
		Dequal,

		LeftParen = 9,
		RightParen = 10,
		Equals,
		Plus,
		Minus,
		Asterisk,
		Slash = 15,
		Dot,
		Comma,
		Colon,
		Semicolon,
		SingleQuote,
		DoubleQuote,
		Percent,
		Tilde,
		BitwiseAnd,
		At,

		Exclamation,

		Pipe, // |
		PipeIn, // <
		PipeOut, // >
		PipeOutAppend, // >>
		Or,
		And,

		Lss,
		Gtr,
		Geq,
		Leq,
		Equ,
		Neq,

		BuiltIn,
		For,
		Do,
		If,
		Else,
		In,
		EnvVar
	};

	int type;
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

private:
	bool eof() { return length == 0 || idx >= length || buffer[idx] == 0; }
	bool isdigit(char c) { return '0' <= c && c <= '9'; }
	bool isalpha(char c) { return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'); }
	bool validIdStart(char c) { return (isalpha(c) || c == '_'); }
	bool validIdBody(char c) { return validIdStart(c) || isdigit(c); }
	bool islxdigit(char c) { return c >= 'a' && c <= 'f'; }
	bool isuxdigit(char c) { return c >= 'A' && c <= 'F'; }
	bool isxdigit(char c) { return islxdigit(c) || isuxdigit(c); }
};
