#pragma once
#include <unordered_map>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <stack>
#include <functional>

// NOTE TO SELF: This doubles down as public API
// so even tho i don't like the placement of things here they have to stay \_(ツ)_/

// todo throw all this into an namespace

#define stringifyfun std::pair<const char *, char *> stringify() override

#define IFUN(name) int name(CallParams callParams)

enum NodeType {
	NODE_LEAF,
	NODE_INNER,
};

enum InnerNodeType {
	INODE_PARENTHESES,
	INODE_BINOP,
	INODE_ASSIGN,
	INODE_COMPARE,
	INODE_IF,
	INODE_FOR
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

	char *srcStart, *srcEnd;
	Node(char type, char *start, char *end);

	typedef Node super;
};

struct LeafNode : public Node {
	union {
		char *str;
		int num;
	};

	using Node::Node;
	typedef LeafNode super;
};

struct InnerNode : public Node {
	union {
		struct { Node **children; int childrenCount; };
		struct { Node *lhs; Node *rhs; };
	};

	using Node::Node;
	typedef InnerNode super;
};

template<typename T, class... types>
struct Evaluatable {
	virtual T evaluate(types... args) = 0;
	typedef T evaltype;
};

struct NumberNode final : public LeafNode {
	NumberNode(int n);
	int evaluate();
	stringifyfun;
};

struct StringNode : public LeafNode {
	StringNode(char *s, bool singlequote = false);
	char *evaluate();
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

struct ParenthesesNode : public InnerNode, public Evaluatable<int, InterpreterState*> {
	int threshold = 16;

	explicit ParenthesesNode();
	void append(Node *n);
	evaltype evaluate(InterpreterState *state);
	stringifyfun;
};

struct BinOpNode : public InnerNode, public Evaluatable<int> {
	explicit BinOpNode(Node *lhs, Node *rhs);
	typedef BinOpNode super;
};

struct AdditionNode final : public BinOpNode {
	using BinOpNode::BinOpNode;
	evaltype evaluate() override;
	stringifyfun;
};

struct SubtractionNode final : public BinOpNode {
	using BinOpNode::BinOpNode;
	evaltype evaluate() override;
	stringifyfun;
};

struct MultiplicationNode final : public BinOpNode {
	using BinOpNode::BinOpNode;
	evaltype evaluate() override;
	stringifyfun;
};

struct DivisionNode final : public BinOpNode {
	using BinOpNode::BinOpNode;
	evaltype evaluate() override;
	stringifyfun;
};

struct EnvVarNode final : public Node {
	char *name, *value = nullptr;

	EnvVarNode(char *name);
	char *evaluate(bool delayedExpansion);
	stringifyfun;
};

struct CallNode final : public Node, public Evaluatable<int, InterpreterState*> {
	char *funcName;
	std::vector<Node *> args;
	bool silent;
	uint32_t hash;

	CallNode(char *name, std::vector<Node*> args, bool slient = false);
	evaltype evaluate(InterpreterState *state);
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

struct CompareNode final : public InnerNode {
	// kinda redundant
	enum class CompareType {
		GTR,
		GEQ,
		LSS,
		LEQ,
		EQU,
		NEQ,
		STRING
	};

	CompareNode(LeafNode *lhs, LeafNode *rhs, CompareType compareType);
	bool evaluate(bool caseInsensitive = false);
	stringifyfun;

//private:
	CompareType cmpType;
	std::function<bool(LeafNode *lhs, LeafNode *rhs, bool caseInsensitive)> cmpFunc;
};

struct ForNode final : public Node, public Evaluatable<int, InterpreterState *> {
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

	ForNode(std::vector<Node*> params);
	int evaluate(InterpreterState *state) override;
	stringifyfun;

//private:
	std::function<bool()> loopCond;
	std::function<void()> inc;
	Node *loopBody;
	std::vector<Node*> params;
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

struct IfNode final : public Node, public Evaluatable<int, InterpreterState*> {
	IfNode(CompareNode *cond, Node *ifBodyRoot, Node *elseBodyRoot = nullptr, bool invert = false, bool caseInsensitive = false);
	evaltype evaluate(InterpreterState *state) override;
	stringifyfun;

//private:
	CompareNode *condition;
	Node *sucess, *failure;
	bool invert, caseInsensitive;
};

struct InterpreterState {
	bool extensions;
	bool delayedExpansion;
	bool echo;
	int filepos;

	std::unordered_map<uint32_t, int> labels;
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
