#pragma once
#include <unordered_map>
#include <cstdint>
#include <cstddef>
#include <vector>

// this header will eventually be public

#define IFUN(name) int name(CallParams callParams)
#define TCAST(type, val) ((type)val)
#define MKNTYP(T, ST) (T | (ST << 1))
#define IN(X) (NODE_INNER | (X << 1))
#define LN(X) (NODE_LEAF | (X << 1))
#define stringifyfun std::pair<const char *, char *> stringify() override

[[gnu::noinline]] constexpr uint64_t hash(const char *text) {
	uint64_t h = 525201411107845655ull, i = 0;
	while (text[i++]) {
		h = (h ^ text[i]) * 0x5bd1e9955bd1e995;
		h ^= h >> 47;
	}
	return h;
}

struct Hasher {
	size_t operator()(const size_t &h) const { return h; }
};

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

struct Node {
	// why the fuck can't bools in c++ hold value greater than 1 even though they occupy 1 byte
	char type; // zzzzzzzzyyyyyyyx => x = NodeType, y = InnerNodeType/LeafNodeType, z = node specific data
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
	void init();
	char *evaluate(bool delayedExpansion);
	stringifyfun;
};

// i have no idea how to implement this.
struct CallNode final : public Node {
	char *funcName;
	std::vector<Node *> args;

	CallNode(char *name, std::vector<Node*> args);
	int execute();
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
};

using CallPtr = int(*)(CallParams callParams);
using RegisterCmdPtr = void(*)(char *cmd, CallPtr fn);