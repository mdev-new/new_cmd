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

// some c++ magic is happening here that i don't understad
// HOW TF DOES THE BINOP NODE KNOW WHAT DERIVED FUNC TO SELECT

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
	NumberNode(int n) {
		this->num = n;
	}

	int evaluate() { return num; }
};

struct StringNode final : public LeafNode {
	StringNode(const char *s) {
		this->str = s;
		this->type = MKNTYP(NODE_LEAF, LNODE_STRING);
	}

	const char *evaluate() { return str; }
};

struct ParenthesesNode final : public InnerNode {
	int threshold = 16;

	ParenthesesNode() {
		this->childrenCount = 0;
		this->children = malloc(threshold*sizeof(Node));
	}
	
	void append(Node *n) {
		if(childrenCount >= threshold) this->children = realloc(this->children, (threshold += 16)*sizeof(Node*));
		children[childrenCount++] = n;
		
		printf("APPEND: %d %d\n", childrenCount, n->type);
	}
	
	int evaluate() {
		// todo implement the algorithm using skip things etc
	}
};

struct BinOpNode : public InnerNode {
	explicit BinOpNode(Node *lhs, Node *rhs) {
		this->lhs = lhs;
		this->rhs = rhs;
		this->type = MKNTYP(NODE_INNER, INODE_BINOP);
	}

	virtual Node* evaluate() = 0;
};

struct AdditionNode final : public BinOpNode {
	using BinOpNode::BinOpNode;

	// todo probably return numbers, not numbernodes
	Node *evaluate() override {
		return new NumberNode(evalExpr(lhs) + evalExpr(rhs));
	}
};

struct SubtractionNode final : public BinOpNode {
	using BinOpNode::BinOpNode;

	Node *evaluate() override {
		return new NumberNode(evalExpr(lhs) - evalExpr(rhs));
	}
};

struct MultiplicationNode final : public BinOpNode {
	using BinOpNode::BinOpNode;

	Node *evaluate() override {
		return new NumberNode(evalExpr(lhs) * evalExpr(rhs));
	}
};

struct DivisionNode final : public BinOpNode {
	using BinOpNode::BinOpNode;

	Node *evaluate() override {
		return new NumberNode(evalExpr(lhs) / evalExpr(rhs));
	}
};

struct EnvVarNode {};

struct ParsedFile {
	Node** nodes;
	int ncount;
};

ParsedFile parse(LexedFile &lexedFile);
