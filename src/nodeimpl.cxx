#include "parser.hpp"
#include "interpreter.hh"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "win.hh"
#include "shared.hh"

#define mkstringify(x, z) std::pair<const char *, char *> x::stringify() { return std::make_pair(#x, z); };

// like interpreter, this file HAS to be optimized aswell

char *itoa_(int i) {
	static char b[21] = {0};
	char *c=b+20;
	int x=abs(i);
	do
		*--c = 48 + x % 10;
	while(x && (x/=10));
	if(i<0) *--c = 45;
	return c;
}

int evalToInt(Node *root, InterpreterState *state = nullptr) {
	switch(root->type & BARETYPE) {
	case MKNTYP(NODE_LEAF, LNODE_NUMBER): return ((NumberNode*)root)->evaluate();
	//case MKNTYP(NODE_INNER, INODE_PARENTHESES): return ((ParenthesesNode*)root)->evaluate();
	case MKNTYP(NODE_INNER, INODE_BINOP): return ((BinOpNode*)root)->evaluate();
	}
}

Node::Node(char type, char *start, char *end)
	: srcStart(start),
	  srcEnd(end),
	  type(type)
{
}

/* === NumberNode === */

NumberNode::NumberNode(int n)
	: super(LN(LNODE_NUMBER), nullptr, nullptr)
{
	this->num = n;
}

int NumberNode::evaluate() { return num; }
mkstringify(NumberNode, itoa_(this->num));

/* === StringNode === */

StringNode::StringNode(char *s, bool singlequote = false)
	: super(LN(LNODE_STRING), nullptr, nullptr),
	singlequote(singlequote)
{
	this->str = strdup(s);
}

char *StringNode::evaluate() { return str; }
mkstringify(StringNode, this->str);

/* === SwitchNode === */
// i am aware this is stupid but i'm doing it for the sake of easier parsing

SwitchNode::SwitchNode(char *s, char pref)
 : StringNode(s),
 prefix(pref)
{
	this->type = MKNTYP(NODE_LEAF, LNODE_SWITCH);
}
mkstringify(SwitchNode, this->str); // todo prepend the prefix to the string

/* === IdNode === */
// i am aware this is stupid but i'm doing it for the sake of easier parsing

IdNode::IdNode(char *s)
 : StringNode(s)
{
	this->type = MKNTYP(NODE_LEAF, LNODE_ID);
}
mkstringify(IdNode, this->str);


/* === ParenthesesNode === */

ParenthesesNode::ParenthesesNode()
	: super(IN(INODE_PARENTHESES) | WITHCHILDREN, nullptr, nullptr)
{
	this->children = malloc(threshold*sizeof(Node*));
	this->childrenCount = 0;
}


void ParenthesesNode::append(Node *n) {
	if(this->childrenCount+1 >= this->threshold) this->children = realloc(this->children, (threshold += 16)*sizeof(Node*));
	children[childrenCount++] = n;

	//printf("APPEND: %d %d\n", childrenCount, n->type);
}

int ParenthesesNode::evaluate(InterpreterState *state) {
	// we'll probably be evaluating commands here aswell

	printf("!!! UNIMPLEMENTED !!!: %s", __PRETTY_FUNCTION__);

	return -1;
}
mkstringify(ParenthesesNode, nullptr);


/* === BinOpNode === */

BinOpNode::BinOpNode(Node *lhs, Node *rhs)
	: InnerNode(IN(INODE_BINOP), nullptr, nullptr)
{
	this->lhs = lhs;
	this->rhs = rhs;
}


/* === AdditionNode === */

int AdditionNode::evaluate() {
	return evalToInt(lhs) + evalToInt(rhs);
}
mkstringify(AdditionNode, itoa_(evaluate()));


/* === SubtractionNode === */

int SubtractionNode::evaluate() {
	return evalToInt(lhs) - evalToInt(rhs);
}
mkstringify(SubtractionNode, itoa_(evaluate()));


/* === MultiplicationNode === */

int MultiplicationNode::evaluate() {
	return evalToInt(lhs) * evalToInt(rhs);
}
mkstringify(MultiplicationNode, itoa_(evaluate()));


/* === DivisionNode === */

int DivisionNode::evaluate() {
	return evalToInt(lhs) / evalToInt(rhs);
}
mkstringify(DivisionNode, itoa_(evaluate()));


/* === EnvVarNode === */

EnvVarNode::EnvVarNode(char *name)
	:super(LN(LNODE_ENVVAR), nullptr, nullptr), 
	 name(name)
{
}

char *EnvVarNode::evaluate(bool delayedExpansion) {
	if(value == nullptr) return value = getenv(name);
	if(!delayedExpansion) return value;
	return getenv(name);
}
mkstringify(EnvVarNode, nullptr);


/* === CallNode === */
CallNode::CallNode(char *name, std::vector<Node*> arguments, bool silent = false)
: super(LN(LNODE_CALL), nullptr, nullptr),
  args(arguments),
  silent(silent)
{
	this->funcName = strdup(name);
}

int CallNode::evaluate(InterpreterState *state) {
	_hashtype_ hashed = _hashfunc_(this->funcName, false);

	// todo stringify whole command

	bool dontEcho = this->silent || state->echo;
	if(!dontEcho) {
	}

	if(multicharMapping.count(hashed) > 0) {
		CallPtr funcPtr = multicharMapping.at(hashed).second;
		if(funcPtr != nullptr) {
			return funcPtr((CallParams){args.data(), args.size(), state});
		}
	} else {
		// todo find the executable
		// and execute with stringified params

		//printf("%s\n", strndup(state->buffer[this->args.front().txtStart], this->args.front().txtStart+this->args.back().txtStart+this->args.back().txtLength));

#ifdef _WIN64
#else
#endif

	}

	return 0;
}
mkstringify(CallNode, this->funcName);


/* === EqualsNode === */
AssignNode::AssignNode(Node *lhs, Node *rhs)
	: super(IN(INODE_ASSIGN), nullptr, nullptr)
{
	this->one.lhs = lhs;
	this->one.rhs = rhs;
}

AssignNode::AssignNode(Node **lhs, int lhsCount, Node *rhs)
	: super(IN(INODE_ASSIGN) | WITHCHILDREN, nullptr, nullptr)
{
	this->many.lhs = lhs;
	this->many.rhs = rhs;
	this->many.lhsCount = lhsCount;
}

void AssignNode::assign() {
	if(this->type & WITHCHILDREN) {
		char *rhstxt = many.rhs->stringify().second;
		for(int i = 0; i < many.lhsCount; i++) setenv(many.lhs[i]->stringify().second, rhstxt, true);
	} else {
		setenv(one.lhs->stringify().second, one.rhs->stringify().second, true);
	}
}

mkstringify(AssignNode, nullptr);


/* === LabelNode === */

LabelNode::LabelNode(char *name, int pos)
 : pos(pos),
 IdNode(name)
{
	this->str = name;
	this->type = MKNTYP(NODE_LEAF, LNODE_LABEL);
}

void LabelNode::_register_(InterpreterState *state) {
	if(!registered) {
		RegisterLabel(state, this);
		registered = true;
	}
	return;
}

mkstringify(LabelNode, this->str);

/* === CompareNode === */

CompareNode::CompareNode(LeafNode *lhs, LeafNode *rhs, CompareType compareType, bool caseInsensitive = false)
	: super(IN(INODE_COMPARE), nullptr, nullptr),
	cmpType(compareType),
	lhs(lhs),
	rhs(rhs)
{
	switch(compareType) {
	case CompareType::GTR: {
		this->cmpFunc = [](LeafNode *lhs, LeafNode *rhs) {
			return lhs->num > rhs->num;
		};
	}
	case CompareType::LSS: {
		this->cmpFunc = [](LeafNode *lhs, LeafNode *rhs) {
			return lhs->num < rhs->num;
		};
	}
	case CompareType::GEQ: {
		this->cmpFunc = [](LeafNode *lhs, LeafNode *rhs) {
			return lhs->num >= rhs->num;
		};
	}
	case CompareType::LEQ: {
		this->cmpFunc = [](LeafNode *lhs, LeafNode *rhs) {
			return lhs->num <= rhs->num;
		};
	}
	case CompareType::EQU: {
		this->cmpFunc = [](LeafNode *lhs, LeafNode *rhs) {
			return lhs->num == rhs->num;
		};
	}
	case CompareType::NEQ: {
		this->cmpFunc = [](LeafNode *lhs, LeafNode *rhs) {
			return lhs->num != rhs->num;
		};
	}
	case CompareType::STRING: {
		this->cmpFunc = [](LeafNode *lhs, LeafNode *rhs) {
			return strcmp(lhs->str, rhs->str) == 0;
		};
	}
	}
}

bool CompareNode::evaluate() {
	bool ret = this->cmpFunc(this->lhs, this->rhs);
	return this->invert? !ret : ret;
}

mkstringify(CompareNode, nullptr);

/* == ForNode == */
// params = /type "opts" ()
ForNode::ForNode(char id, std::vector<Node*> params, Node *loopBody)
	: super(IN(INODE_FOR), nullptr, nullptr),
	id(id),
	loopBody(loopBody)
{
	if(params[0]->type == LN(LNODE_SWITCH)) {
		SwitchNode *sw = params[0];
		if(strcmp(sw->str, "L") == 0) this->forType = ForType::NUMBERS;
		else if(strcmp(sw->str, "R") == 0) this->forType = ForType::FILESROOTED;
		else if(strcmp(sw->str, "D")) this->forType = ForType::FOLDERS;
		else if(strcmp(sw->str, "F")) {
			ParenthesesNode *pn = params[2];
			if(pn->children[0]->type == LNODE_STRING) {
				StringNode *str = pn->children[0];
				if(str->singlequote) this->forType = ForType::CMDRESULTS;
				else this->forType = ForType::STRINGCONTENTS;
			} else {
				this->forType = ForType::FILECONTENTS;
			}
		}
	} else {
		this->forType = ForType::FILES;
	}

	switch(this->forType) {
	case ForType::FILES: {
		this->loopCond = [this]() { return true; };
		break;
	}
	case ForType::FILESROOTED: {
		this->loopCond = [this]() { return true; };
		break;
	}
	case ForType::FOLDERS: {
		this->loopCond = [this]() { return true; };
		break;
	}
	case ForType::NUMBERS: {
		this->loopCond = [this]() { return this->range.current <= this->range.end; };
		this->inc = [this]() { this->range.current++; };
		break;
	}
	case ForType::FILECONTENTS: {
		this->loopCond = [this]() { return true; };
		break;
	}
	case ForType::STRINGCONTENTS: {
		this->loopCond = [this]() { return true; };
		break;
	}
	case ForType::CMDRESULTS: {
		this->loopCond = [this]() { return true; };
		break;
	}
	}
}

int ForNode::evaluate(InterpreterState *state) {
	for(; this->loopCond(); this->inc()) {
		((Evaluatable<int, InterpreterState *>*)this->loopBody)->evaluate(state);
	}

	return 0;
}

mkstringify(ForNode, nullptr);

IfNode::IfNode(CompareNode *cond, Node *ifBodyRoot, Node *elseBodyRoot = nullptr, bool invert = false, bool caseInsensitive = false)
	: super(IN(INODE_IF), nullptr, nullptr),
	condition(cond),
	sucess(ifBodyRoot),
	failure(elseBodyRoot)
{

}

int IfNode::evaluate(InterpreterState *state) {
	if(condition->evaluate()) {
		((Evaluatable<int, InterpreterState*>*)sucess)->evaluate(state);
	} else {
		((Evaluatable<int, InterpreterState*>*)failure)->evaluate(state);
	}

	return 0;
}

mkstringify(IfNode, nullptr);