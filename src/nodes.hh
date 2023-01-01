#pragma once
#include <unordered_map>
#include <cstdint>
#include <cstddef>
#include <vector>

// this header will eventually be public

#define stringifyfun std::pair<const char *, char *> stringify() override

enum NodeType {
	NODE_LEAF,
	NODE_INNER,
};

enum InnerNodeType {
	INODE_PARENTHESES,
	INODE_BINOP,
	INODE_ASSIGN,
};

enum LeafNodeType {
	LNODE_NUMBER,
	LNODE_STRING,
	LNODE_ENVVAR,
	LNODE_CALL,
	LNODE_LABEL,
	LNODE_ID,
	LNODE_SWITCH,
};

struct InterpreterState;
struct Node {
	// zzzzzzzzyyyyyyyx => x = NodeType, y = InnerNodeType/LeafNodeType, z = node specific data
	char type;
	virtual std::pair<const char *, char *> stringify() = 0;
};

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
	stringifyfun;
};

struct StringNode : public LeafNode {
	StringNode(char *s);
	char *evaluate();
	stringifyfun;
};

struct IdNode final : public StringNode {
	IdNode(char *s);
	stringifyfun;
};

struct SwitchNode final : public StringNode {
	char prefix;
	SwitchNode(char *s, char pref);
	stringifyfun;
};

struct ParenthesesNode : public InnerNode {
	int threshold = 16;

	explicit ParenthesesNode();
	void append(Node *n);
	int evaluate();
	stringifyfun;
};

struct BinOpNode : public InnerNode {
	explicit BinOpNode(Node *lhs, Node *rhs);
	virtual int evaluate() = 0;
};

struct AdditionNode final : public BinOpNode {
	using BinOpNode::BinOpNode;
	int evaluate() override;
	stringifyfun;
};

struct SubtractionNode final : public BinOpNode {
	using BinOpNode::BinOpNode;
	int evaluate() override;
	stringifyfun;
};

struct MultiplicationNode final : public BinOpNode {
	using BinOpNode::BinOpNode;
	int evaluate() override;
	stringifyfun;
};

struct DivisionNode final : public BinOpNode {
	using BinOpNode::BinOpNode;
	int evaluate() override;
	stringifyfun;
};

struct EnvVarNode final : public Node {
	char *name, *value = nullptr;

	EnvVarNode(char *name);
	char *evaluate(bool delayedExpansion);
	stringifyfun;
};

// i have no idea how to implement this.
struct CallNode final : public Node {
	char *funcName;
	std::vector<Node *> args;

	CallNode(char *name, std::vector<Node*> args);
	int execute(InterpreterState *state);
	stringifyfun;
};

struct AssignNode final : public Node {
	union {
		struct { Node *lhs, *rhs; } one;
		struct { Node **lhs; Node *rhs; int lhsCount; } many;
	};

	AssignNode(Node *lhs, Node *rhs);
	AssignNode(Node **lhs, int lhsCount, Node *rhs);
	void assign();
	stringifyfun;
};

struct LabelNode final : public Node {
	int filePos;
	LabelNode(int pos);
	stringifyfun;
};

struct CallParams {
	Node **params;
	int noParams;
	InterpreterState *state;
};

using CallPtr = int(*)(CallParams callParams);
using RegisterCmdPtr = void(*)(char *cmd, CallPtr fn);