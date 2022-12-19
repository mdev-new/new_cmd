#pragma once
#include "lexer.hpp"
#include <cstring>
#include <vector>
#include <memory>
#include <typeinfo>

#define MKNTYP(NT,ST) (NT | (ST << 1))

// original thought was to compile the whole program
// into series of nodes that would get evaluated at runtime

// a part of interpreter is already here

// todo migrate to separate file

// TODO EXPRESSION TREE

// I hate that this has to be here but whatever
enum NodeType {
	NODE_LEAF,
	NODE_INNER,
};

enum InnerNodeType {
	INODE_PARENTHESES,
	INODE_BINOP
};

enum LeafNodeType {
	LNODE_NUMBER,
	LNODE_STRING
};

struct Node {
	uint16_t type; // .......yyyyyyyyx => x = NodeType, y = InnerNodeType/LeafNodeType
};

int evalExpr(Node *root);

struct LeafNode : public Node {
	union {
		const char *str;
		int num;
	};
};

struct InnerNode : public Node {
	union {
		struct { Node **children; int childrenCount; };
		struct { Node *lhs; Node *rhs; };
	};
};

struct NumberNode final : public LeafNode {
	NumberNode(int n);
	int evaluate();
};

struct StringNode final : public LeafNode {
	StringNode(const char *s);
	const char *evaluate();
};

struct ParenthesesNode final : public InnerNode {
	int threshold = 16;

	ParenthesesNode();
	void append(Node *n);
	int evaluate();
};

struct BinOpNode : public InnerNode {
	explicit BinOpNode(Node *lhs, Node *rhs);
	virtual int evaluate() = 0;
};

struct AdditionNode final : public BinOpNode {
	using BinOpNode::BinOpNode;
	int evaluate() override;
};

struct SubtractionNode final : public BinOpNode {
	using BinOpNode::BinOpNode;
	int evaluate() override;
};

struct MultiplicationNode final : public BinOpNode {
	using BinOpNode::BinOpNode;
	int evaluate() override;
};

struct DivisionNode final : public BinOpNode {
	using BinOpNode::BinOpNode;
	int evaluate() override;
};

struct EnvVarNode final : public Node {
	char *name;
	char *value;

	EnvVarNode(const char *name);
	void init();
	const char *evaluate(bool delayedExpansion);
};

// i have no idea how to implement this.
struct CallNode final : public Node {
};

struct ParsedFile {
	Node** nodes;
	int ncount;
};

ParsedFile parse(LexedFile &lexedFile);
