#pragma once
#include "lexer.hpp"
#include <cstring>

// I hate that this has to be here but whatever
enum NodeType {
	NODE_PROGRAM,
	NODE_NUMBER,
	NODE_PLUS,
	NODE_MINUS,
	NODE_CALL,
};

struct Node {
	NodeType type;
};

struct ProgramNode final : public Node {

};

struct BinOpNode : public Node {
	Node* left;
	Node* right;
};

// so far only ints
struct NumberNode final : public Node {
	int value;
	NumberNode(int num) {
		Node::type = NODE_NUMBER;
		value = num;
	}
};

struct PlusNode final : public BinOpNode {};
struct MinusNode final : public BinOpNode {};

struct ParsedFile {
	Node rootNode;
};

ParsedFile parse(LexedFile &lexedFile);
