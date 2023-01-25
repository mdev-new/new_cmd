#pragma once
#include <unordered_map>
#include <cstdint>
#include <cstdio>
#include <cstddef>
#include <vector>
#include <stack>
#include <functional>
#include <variant>

// NOTE TO SELF: This doubles down as public API
// so even tho i don't like the placement of things here they have to stay \_(ツ)_/

// todo throw all this into an namespace

#define stringifyfun std::pair<const char *, char *> stringify() override
#define IFUN(name) int name(CallParams callParams)

#define NTP(x, y) (x | (y << 1))

// todo lowercase
struct NodeType {
	enum {
		Parentheses = NTP(0, 0),
		BinOp		= NTP(0, 1),
		Assign		= NTP(0, 2),
		Compare		= NTP(0, 3),
		If			= NTP(0, 4),
		For			= NTP(0, 5),
		Call		= NTP(0, 6),

		Number		= NTP(1, 1),
		String		= NTP(1, 2),
		EnvVar		= NTP(1, 3),
		Label		= NTP(1, 4),
		Id			= NTP(1, 5),
		Switch		= NTP(1, 6),
	};
};

struct InterpreterState;
struct Node {
	short type; //zzzzzzzzzzyyyyyx // z=custom storage
	virtual std::pair<const char *, char *> stringify() = 0;

	size_t srcStart, srcEnd;
	Node(char type, char *start, char *end);

	typedef Node super;
};

struct EvaluatableNode : public Node {
	virtual uint64_t evaluate(InterpreterState *s = nullptr);
	using Node::Node;
	typedef EvaluatableNode super;

	virtual operator int() const;
};

struct LeafNode : public Node {
	union {
		char *str;
		int num;
	};

	using Node::Node;
	typedef LeafNode super;
};

template<typename T = Node*>
struct InnerNode : public Node {
	union {
		struct { std::vector<T>* children; };
		struct { T lhs; T rhs; };
	};

	using Node::Node;
	typedef InnerNode super;
};

struct NumberNode final : public LeafNode {
	NumberNode(int n);
	uint64_t evaluate();
	stringifyfun;
};

struct StringNode : public LeafNode {
	StringNode(char *s, bool singlequote = false);
	bool singlequote = false;
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

struct ParenthesesNode : public InnerNode<EvaluatableNode*> {
	ParenthesesNode();
	uint64_t evaluate(InterpreterState *state);
	stringifyfun;
};

struct BinOpNode : public InnerNode<EvaluatableNode*> {
	BinOpNode(Node *lhs, Node *rhs);
	virtual int evaluate() = 0;
	typedef BinOpNode super;
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

struct EnvVarNode final : public EvaluatableNode {
	char *name, *value = nullptr;
	bool delayedExpansion;

	EnvVarNode(char *name, bool delayedExpansion);
	char *evaluate();
	stringifyfun;

	operator int() const;
};

struct CallNode final : public EvaluatableNode {
	char *funcName;
	bool silent;
	uint32_t hash;
	std::vector<Node*> *children;

	CallNode(char *name, std::vector<Node*> args, bool slient = false);
	uint64_t evaluate(InterpreterState *state) override;
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
	LabelNode(char *name, int pos);
	stringifyfun;
};

struct CompareNode final : public InnerNode<LeafNode*> {
	// kinda redundant
	enum class CompareType {
		GTR,
		GEQ,
		LSS,
		LEQ,
		EQU,
		NEQ,
		STRING,
		DEFINED,
		EXISTS
	};

	CompareNode(LeafNode *lhs, LeafNode *rhs, CompareType compareType);
	uint64_t evaluate(bool caseInsensitive = false);
	stringifyfun;

//private:
	CompareType cmpType;
	std::function<bool(LeafNode *lhs, LeafNode *rhs, bool caseInsensitive)> cmpFunc;
};

struct ForNode final : public EvaluatableNode {
	// completely redundantant but whatever
	enum class ForType {
		FILES, // for
		FILESROOTED, // for /r
		FOLDERS, // for /d
		NUMBERS, // for /l
		FILECONTENTS, // for /f (file1 file2)
		STRINGCONTENTS, // for /f ("string to process")
		CMDRESULTS // for /f ('command to be ran')
	} forType;

	ForNode(ForType type, char id, ParenthesesNode *cond, EvaluatableNode *body);
	uint64_t evaluate(InterpreterState *state);
	stringifyfun;

//private:
	std::function<bool()> loopCond;
	std::function<void()> inc;
	EvaluatableNode *loopBody;
	char id;

	union {
		struct { int start, step, end, current; } range;
		struct { char *path, *filenameset; } filesrootedat;
		struct { char **folders; } dirs;
		struct { char *options, **filenames; int filenameCount; } files;
		struct { char *options, *string; } string;
		struct { char *options, *command; } commandoutput;
	};
};

struct IfNode final : public EvaluatableNode {
	IfNode(CompareNode *cond, EvaluatableNode *ifBodyRoot, EvaluatableNode *elseBodyRoot = nullptr, bool invert = false, bool caseInsensitive = false);
	uint64_t evaluate(InterpreterState *state) override;
	stringifyfun;

//private:
	CompareNode *condition;
	EvaluatableNode *sucess, *failure;
	bool invert, caseInsensitive;
};

struct InterpreterState {
	bool echo;
	int filepos;

	std::unordered_map<uint32_t, int> labels;
	std::stack<char*> directoryStack;
	std::stack<std::unordered_map<char *, char *>> localEnvironments; //todo every call level recieves fresh one, so that means a stack of stacks?
	char *buffer;
	size_t bufferSize;
};

struct CallParams {
	std::vector<Node*>* params;
	InterpreterState *state;
};

using CallPtr = int(*)(CallParams callParams);
using RegisterCmdPtr = void(*)(char *cmd, CallPtr fn);
