#pragma once
#include <unordered_map>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <stack>

// NOTE TO SELF: This doubles down as public API
// so even tho i don't like the placement of things here they have to stay \_(ツ)_/

#define stringifyfun std::pair<const char *, char *> stringify() override

#define _hashfunc_(x) hash32(x)
#define _hashtype_ uint32_t

#define IFUN(name) int name(CallParams callParams)

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
struct Token;
struct Node {
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

struct IdNode : public StringNode {
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
	bool silent;

	CallNode(char *name, std::vector<Node*> args, bool slient = false);
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

struct LabelNode final : public IdNode {
	int pos;
	bool registered;
	LabelNode(char *name, int pos);
	void _register_(InterpreterState *state);
	stringifyfun;
};

struct InterpreterState {
	bool extensions;
	bool delayedExpansion;
	bool echo;
	int filepos;

	std::unordered_map<_hashtype_, int> labels;
	std::stack<char*> directoryStack;
	std::stack<std::unordered_map<char *, char *>> localEnvironments; //todo every call level recieves fresh one, so that means a stack of stacks?
	char *buffer;
	size_t bufferSize;
};

struct CallParams {
	Node **params;
	int noParams;
	InterpreterState *state;
};

using CallPtr = int(*)(CallParams callParams);
using RegisterCmdPtr = void(*)(char *cmd, CallPtr fn);
