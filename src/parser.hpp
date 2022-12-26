#pragma once
#include "lexer.hpp"
#include <cstring>
#include <vector>
#include <memory>
#include <typeinfo>
#include "parser_interpreter_shared.hh"

#define MKNTYP(NT,ST) (NT | (ST << 1))
#define WITHCHILDREN (1 << 15)
#define BARETYPE (0xFF)

// TODO EXPRESSION TREE

// I hate that this has to be here but whatever
enum NodeType {
	NODE_LEAF,
	NODE_INNER,
};

enum InnerNodeType {
	INODE_PARENTHESES,
	INODE_BINOP,
	INODE_ASSIGN
};

enum LeafNodeType {
	LNODE_NUMBER,
	LNODE_STRING,
	LNODE_ENVVAR,
	LNODE_CALL,
	LNODE_LABEL,
};

struct Node {
	uint16_t type; // zzzzzzzzyyyyyyyx => x = NodeType, y = InnerNodeType/LeafNodeType, z = node specific data
	virtual std::pair<const char *, char *> stringify() = 0;
};

// implemented in nodeimpl.cxx
int evalExpr(Node *root);

struct LeafNode : public Node {
	union {
		char *str;
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
	std::pair<const char *, char *> stringify() override;
};

struct StringNode final : public LeafNode {
	StringNode(const char *s);
	const char *evaluate();
	std::pair<const char *, char *> stringify() override;
};

struct ParenthesesNode : public InnerNode {
	int threshold = 16;

	explicit ParenthesesNode();
	void append(Node *n);
	int evaluate();
	std::pair<const char *, char *> stringify() override;
};

struct BinOpNode : public InnerNode {
	explicit BinOpNode(Node *lhs, Node *rhs);
	virtual int evaluate() = 0;
};

struct AdditionNode final : public BinOpNode {
	using BinOpNode::BinOpNode;
	int evaluate() override;
	std::pair<const char *, char *> stringify() override;
};

struct SubtractionNode final : public BinOpNode {
	using BinOpNode::BinOpNode;
	int evaluate() override;
	std::pair<const char *, char *> stringify() override;
};

struct MultiplicationNode final : public BinOpNode {
	using BinOpNode::BinOpNode;
	int evaluate() override;
	std::pair<const char *, char *> stringify() override;
};

struct DivisionNode final : public BinOpNode {
	using BinOpNode::BinOpNode;
	int evaluate() override;
	std::pair<const char *, char *> stringify() override;
};

struct EnvVarNode final : public Node {
	char *name, *value = nullptr;

	EnvVarNode(char *name);
	void init();
	char *evaluate(bool delayedExpansion);
	std::pair<const char *, char *> stringify() override;
};

// i have no idea how to implement this.
struct CallNode final : public Node {
	char *funcName;
	std::vector<Node *>args;

	CallNode(char *name, std::vector<Node*> args);
	int execute();
	std::pair<const char *, char *> stringify() override;
};

struct AssignNode final : public Node {
	union {
		struct { Node *lhs, *rhs; } _1x1;
		struct { Node **lhs; Node *rhs; int lhsCount; } _Xx1;
	};

	AssignNode(Node *lhs, Node *rhs);
	AssignNode(Node **lhs, int lhsCount, Node *rhs);
	std::pair<const char *, char *> stringify() override;
};

struct LabelNode final : public Node {
	int filePos;
	LabelNode(int pos);
	std::pair<const char *, char *> stringify() override;
};

class Parser {
	Lexer lexer;

public:
	Parser(char *buffer, size_t length);
	void parse();

	std::vector<Node*> nodes;
};
