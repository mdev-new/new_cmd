#pragma once
#include <unordered_map>
#include <cstdint>
#include <cstddef>
#include <vector>

// this header will eventually be public

#define IFUN(name) int name(CallParams callParams)
#define TCAST(type, val) ((type)val)
#define MKNTYP(T, ST) (T | (ST << 1))

[[gnu::noinline]] constexpr uint64_t hash(const char *text) {
	uint64_t h = 525201411107845655ull, i = 0;
	while (text[i++]) {
		h = (h ^ text[i]) * 0x5bd1e9955bd1e995;
		h ^= h >> 47;
	}
	return h;
}

// lets do little bit of trolling
// to avoid double hashing
struct Hasher {
	size_t operator()(const size_t &h) const {
		return h;
	}
};

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
	bool type; // zzzzzzzzyyyyyyyx => x = NodeType, y = InnerNodeType/LeafNodeType, z = node specific data
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
	std::pair<const char *, char *> stringify() override;
};

struct StringNode final : public LeafNode {
	StringNode(char *s);
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
	std::vector<Node *> args;

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

struct CallParams {
	Node **params;
	int paramLen;
};
using CallPtr = int(*)(CallParams callParams);

// mapping of strings to tokens & functions:
// volatile std::unordered_map<size_t, std::pair<uint16_t, CallPtr>, Hasher> multicharMapping;
