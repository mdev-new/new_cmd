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

#define TCAST(type, val) ((type)val)
#define WITHCHILDREN (1 << 15)
#define WITHCHILDREN_(N) (N & (1 << 15))
#define BARETYPE 0xF
#define BARETYPE_(N) (N & 0xF)

#define NTP(x, y) (x | (y << 1))

struct InterpreterState {
	bool echo;
	int filepos;

	std::unordered_map<uint32_t, int> labels;
	std::stack<char*> directoryStack;
	std::stack<std::unordered_map<char *, char *>> localEnvironments; //todo every call level recieves fresh one, so that means a stack of stacks?
	char *buffer;
	size_t bufferSize;
};

struct Node;
struct CallParams {
	std::vector<Node*>* params;
	InterpreterState *state;
	FILE *fpRead, *fpWrite;
};

using CallPtr = int(*)(CallParams callParams);

struct Node {
	enum Type {
		Parentheses		= NTP(1, 0),
		BinOp			= NTP(1, 1),
		Assign			= NTP(1, 2),
		Compare			= NTP(1, 3),
		If				= NTP(1, 4),
		For				= NTP(1, 5),
		Call			= NTP(1, 6),

		Number			= NTP(0, 0),
		String			= NTP(0, 1),
		EnvVar			= NTP(0, 2),
		Label			= NTP(0, 3),
		Id				= NTP(0, 4),
		Switch			= NTP(0, 5),
	};

	uint16_t type; //zzzzzzzzzzyyyyyx // z=custom storage
	virtual std::pair<const char *, char *> stringify() = 0;

	uint64_t evaluate(InterpreterState *state = nullptr);
	virtual operator int() const;

	union {
		union {
			struct { std::vector<Node*>* children; };
			struct { Node *lhs, *rhs; };
		};

		union {
			char *str;
			int num;
		};
	};

	size_t srcStart, srcEnd;
	Node(decltype(type) type, char *start, char *end);

	typedef Node super;
};

struct NumberNode final : public Node {
	NumberNode(int n);
	stringifyfun;
	operator int() const override;
};

struct EnvVarNode;
struct StringNode : public Node {
	StringNode(char *s, bool singlequote = false, std::vector<EnvVarNode*> *substitutions = nullptr);
	bool singlequote = false;
	stringifyfun;

private:
	std::vector<EnvVarNode*> *substitutions;
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

struct ParenthesesNode : public Node {
	ParenthesesNode();
	uint64_t evaluate(InterpreterState *state);
	stringifyfun;
};

struct BinOpNode : public Node {
	BinOpNode(Node *lhs, Node *rhs);
	virtual int evaluate() = 0;
	operator int() const override;
	typedef BinOpNode super;
};

#define MakeBinOp(name) \
struct name final : public BinOpNode {\
	using BinOpNode::BinOpNode;\
	int evaluate() override;\
	stringifyfun;\
}

MakeBinOp(AdditionNode);
MakeBinOp(SubtractionNode);
MakeBinOp(DivisionNode);
MakeBinOp(MultiplicationNode);
MakeBinOp(BinaryOrNode);
MakeBinOp(BinaryAndNode);
MakeBinOp(BinaryNot);
MakeBinOp(BinaryModulo);
MakeBinOp(BinaryXor);
MakeBinOp(LogicalOrNode);
MakeBinOp(LogicalAndNode);
MakeBinOp(LogicalNot);

#undef MakeBinOp

struct EnvVarNode final : public Node {
	char *name, *value = nullptr;
	bool delayedExpansion;

	EnvVarNode(char *name, bool delayedExpansion);
	char *evaluate();
	stringifyfun;

	operator int() const override;
};

struct CallNode final : public Node {
	char *funcName;
	bool silent;
	uint32_t hash;

	CallNode(char *name, std::vector<Node*> args, bool slient = false);
	int evaluate(InterpreterState *state);
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

struct CompareNode final : public Node {
	// kinda redundant
	enum class CompareType {
		Greater,
		GreaterOrEqual,
		Less,
		LessOrEqual,
		Equal,
		NotEqual,
		String,
		Defined,
		Exists
	};

	CompareNode(Node *lhs, Node *rhs, CompareType compareType);
	bool evaluate(bool caseInsensitive = false);
	stringifyfun;

//private:
	CompareType cmpType;
	std::function<bool(Node *lhs, Node *rhs, bool caseInsensitive)> cmpFunc;
};

struct ForNode final : public Node {
	// completely redundantant but whatever
	enum class ForType {
		Files, // for
		FilesRooted, // for /r
		Folders, // for /d
		Numbers, // for /l
		FileContents, // for /f (file1 file2)
		StringContents, // for /f ("string to process")
		CmdResults // for /f ('command to be ran')
	} forType;

	ForNode(ForType type, char id, ParenthesesNode *cond, Node *body, StringNode *opts = nullptr);
	uint64_t evaluate(InterpreterState *state);
	stringifyfun;

//private:
	std::function<bool()> loopCond;
	std::function<void()> inc;
	std::function<void()> setEnv;
	Node *loopBody;
	StringNode *opts; // todo remove, further parse
	ParenthesesNode *cond; //todo remove
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

struct IfNode final : public Node {
	IfNode(CompareNode *cond, Node *ifBodyRoot, Node *elseBodyRoot = nullptr, bool invert = false, bool caseInsensitive = false);
	int evaluate(InterpreterState *state);
	stringifyfun;

//private:
	CompareNode *condition;
	Node *sucess, *failure;
	bool invert, caseInsensitive;
};
