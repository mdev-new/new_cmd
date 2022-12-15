#pragma once
#include "lexer.hpp"
#include <cstring>
#include <vector>
#include <memory>
#include <typeinfo>

// original thought was to compile the whole program
// into series of nodes that would get evaluated at runtime

// I hate that this has to be here but whatever
enum NodeType {
	NODE_LEAF,
	NODE_INNER,
};

enum InnerNodeType {
	INODE_PARENTHESES,
	INODE_ADD,
	INODE_SUB,
	INODE_MUL,
	INODE_DIV
};

enum LeafNodeType {
	LNODE_NUMBER,
	LNODE_STRING
};

struct Node {
	uint16_t type; // .......yyyyyyyyx => x = NodeType, y = InnerNodeType/LeafNodeType
};

struct LeafNode : public Node {
	union {
		const char *str;
		int num;
	};
};

struct InnerNode : public Node {
protected:
	union {
		struct { Node **children; int childrenCount; };
		struct { Node *lhs; Node *rhs; };
	};
};

struct NumberNode : public LeafNode {
	NumberNode(int n) {
		this->num = n;
	}

	int evaluate() { return num; }
};

struct StringNode : public LeafNode {
	StringNode(const char *s) {
		this->str = s;
		this->type = NODE_LEAF | (LNODE_STRING << 1);
	}

	const char *evaluate() { return str; }
};

struct ParenthesesNode : public InnerNode {
	void append(Node *n) {
		children[childrenCount++] = n;
	}
};

struct BinOpNode : public InnerNode {
	BinOpNode(Node *lhs, Node *rhs) {
		this->lhs = lhs;
		this->rhs = rhs;
	}

	virtual Node* evaluate();
};

struct AdditionNode final : public BinOpNode {
	AdditionNode(Node *lhs, Node *rhs)
	:	BinOpNode(lhs, rhs)
	{
		this->type = NODE_INNER | (INODE_ADD << 1);
	}

	Node *evaluate() override {
		int e_lhs = 0;
		int e_rhs = 0;

		if(lhs->type == (NODE_LEAF | (LNODE_NUMBER << 1)))
			e_lhs = ((NumberNode*)lhs)->evaluate();

		//else if(lhs->type == (NODE_INNER | (INODE_PARENTHESES << 1)))
		//	e_lhs = ((NumberNode*)lhs)->evaluate()

		if(rhs->type == (NODE_LEAF | (LNODE_NUMBER << 1)))
			e_rhs = ((NumberNode*)rhs)->evaluate();

		return new NumberNode(e_lhs + e_rhs);
	}
};

struct SubtractionNode final : public BinOpNode {
	SubtractionNode(Node *lhs, Node *rhs)
	:	BinOpNode(lhs, rhs)
	{
		this->type = NODE_INNER | (INODE_SUB << 1);
	}
};

struct MultiplicationNode final : public BinOpNode {
	MultiplicationNode(Node *lhs, Node *rhs)
	:	BinOpNode(lhs, rhs)
	{
		this->type = NODE_INNER | (INODE_MUL << 1);
	}
};

struct DivisionNode final : public BinOpNode {
	DivisionNode(Node *lhs, Node *rhs)
	:	BinOpNode(lhs, rhs)
	{
		this->type = NODE_INNER | (INODE_DIV << 1);
	}
};

struct EnvVarNode {};

struct ParsedFile {
	Node** nodes;
	int ncount;
};

ParsedFile parse(LexedFile &lexedFile);
