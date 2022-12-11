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
	virtual char* stringify() {}
};

// todo move implementations to separate file

struct ProgramNode final : public Node {
	Node **body;
	int nodeCount;

	ProgramNode(Node **b, int c) {
		body = b;
		nodeCount = c;
		type = NODE_PROGRAM;
	}
};

struct BinOpNode : public Node {
	Node* left;
	Node* right;
};

// so far only ints
struct NumberNode final : public Node {
	int value;
	NumberNode(int num) {
		this->type = NODE_NUMBER;
		value = num;
	}
};

// todo (plus + minus) maybe return NumberNode?
struct PlusNode final : public BinOpNode {
	PlusNode(Node *left, Node *right) {
		this->left = left;
		this->right = right;
		this->type = NODE_PLUS;
	}

	int evaluate() {
		if(left->type == right->type == NODE_NUMBER) return (*(NumberNode*)left).value + (*(NumberNode*)right).value;
		else return 0;
	}
};
struct MinusNode final : public BinOpNode {
	MinusNode(Node *left, Node *right) {
		this->left = left;
		this->right = right;
		this->type = NODE_MINUS;
	}

	int evaluate() {
		if(left->type == right->type == NODE_NUMBER) return (*(NumberNode*)left).value - (*(NumberNode*)right).value;
		else return 0;
	}
};

struct ExeCallNode final : public Node {};

// todo figure out how to implement internal commands with special handling (for, do, if, else, set, etc)

struct SetNode final : public Node {};

struct ParsedFile {
	ProgramNode *rootNode;
};

ParsedFile parse(LexedFile &lexedFile);
