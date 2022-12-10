#pragma once
#include "lexer.hpp"
#include <cstring>

struct Number {
	union {
		float f;
		int i;
	};
};

struct Value {
	union {
		Number num;
		char *str;
	};

	Value() {};
	Value(int i) { num.i = i; }
	Value(float f) { num.f = f; }
	Value(char *string) { str = string; }
};

enum NodeType {
	NODE_NOVAL,
	NODE_NUMBER,
	NODE_STRING,
	NODE_LABEL,
	NODE_ENVVAR,
	NODE_BUILTIN,
	NODE_PLUS_NOVAL,
	NODE_MINUS_NOVAL,
};

struct Node {
	Node *children;
	int noOfChildren;

	Value value;
	NodeType valueType;
};

struct ParsedFile {
	Node rootNode;
};

ParsedFile parse(LexedFile &lexedFile);
