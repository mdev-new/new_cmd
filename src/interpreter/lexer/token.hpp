#pragma once

struct Token {
	enum Type {
		Inval = -1,
		Eof = 0,
		Space,
		Id,
		Number,
		String,
		SingleQuoteString,

		// created artificially by parser
		Switch = 6,
		Label,
		// ---

		Dequal,

		LeftParen,
		RightParen,
		Dot,
		Comma,
		Colon,
		Semicolon,
		SingleQuote,
		DoubleQuote,
		At,

		Exclamation,
		Equals,
		Percent,
		Tilde,

		Pipe, // |
		PipeIn, // <
		PipeOut, // >
		PipeOutAppend, // >>

		Plus,
		Minus,
		Asterisk,
		Slash,
		And,
		Caret,
		LogicalOr,
		LogicalAnd,
		
		Lss,
		Gtr,
		Geq,
		Leq,
		Equ,
		Neq,
		PlusEquals,
		MinusEquals,
		TimesEquals,
		DivideEquals,
		OrEquals,
		AndEquals,
		XorEquals,

		EnvVar,

		BuiltIn,
		For,
		Do,
		If,
		Else,
		In
	};

	int type;
	uint64_t value;
	uint64_t additionalData;

	int txtStart;
	int txtLength;
};