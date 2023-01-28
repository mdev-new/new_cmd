#include "parser.hpp"
#include "interpreter.hh"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "win.hh"
#include "shared.hh"

#include "commands.hh"

#define mkstringify(x, z) std::pair<const char *, char *> x::stringify() { return std::make_pair(#x, z); };

// like interpreter, this file HAS to be optimized aswell
// atleast the things on hot path

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

Node::Node(decltype(Node::type) type, char *start, char *end)
	: srcStart(start),
	  srcEnd(end),
	  type(type)
{
}

uint64_t Node::evaluate(InterpreterState *s = nullptr) {
	switch(this->type & BARETYPE) {
	case Node::Type::If: return TCAST(IfNode*, this)->evaluate(s);
	case Node::Type::For: return TCAST(ForNode*, this)->evaluate(s);
	case Node::Type::Call: return TCAST(CallNode*, this)->evaluate(s);
	case Node::Type::BinOp: return TCAST(BinOpNode*, this)->evaluate();
	case Node::Type::Number: return TCAST(NumberNode*, this)->num;
	default: printf("hit default! %d\n", this->type); return 0;
	}
}

Node::operator int() const {
	switch(this->type) {
	case Node::Type::EnvVar: return int(*TCAST(EnvVarNode*, this));
	default: return 0;
	}
}

/* === NumberNode === */

NumberNode::NumberNode(int n)
	: super(Node::Type::Number, 0, 0)
{
	this->num = n;
}

mkstringify(NumberNode, itoa_(this->num));

/* === StringNode === */

StringNode::StringNode(char *s, bool singlequote = false)
	: super(Node::Type::String, 0, 0),
	singlequote(singlequote)
{
	this->str = strdup(s);
}

mkstringify(StringNode, this->str);

/* === SwitchNode === */
// i am aware this is stupid but i'm doing it for the sake of easier parsing

SwitchNode::SwitchNode(char *s, char pref)
 : StringNode(s),
 prefix(pref)
{
	this->type = Node::Type::Switch;
}
mkstringify(SwitchNode, this->str); // todo prepend the prefix to the string

/* === IdNode === */
// i am aware this is stupid but i'm doing it for the sake of easier parsing

IdNode::IdNode(char *s)
 : StringNode(s)
{
	this->type = Node::Type::Id;
}
mkstringify(IdNode, this->str);


/* === ParenthesesNode === */

ParenthesesNode::ParenthesesNode()
	: super(Node::Type::Parentheses | WITHCHILDREN, 0, 0)
{
	this->children = new std::vector<Node*>();
}

uint64_t ParenthesesNode::evaluate(InterpreterState *state) {
	int retcode = 0;

	auto& children = *this->children;

	for(int i = 0; i < children.size(); i++) {
		retcode = children.at(i)->evaluate(state);
	}

	return retcode;
}
mkstringify(ParenthesesNode, nullptr);


/* === BinOpNode === */

BinOpNode::BinOpNode(Node *lhs, Node *rhs)
	: Node(Node::Type::BinOp, 0, 0)
{
	this->lhs = lhs;
	this->rhs = rhs;
}


/* === AdditionNode === */

int AdditionNode::evaluate() {
	return int(lhs->evaluate()) + int(rhs->evaluate());
}
mkstringify(AdditionNode, itoa_(evaluate()));


/* === SubtractionNode === */

int SubtractionNode::evaluate() {
	return int(lhs->evaluate()) - int(rhs->evaluate());
}
mkstringify(SubtractionNode, itoa_(evaluate()));


/* === MultiplicationNode === */

int MultiplicationNode::evaluate() {
	return int(lhs->evaluate()) * int(rhs->evaluate());
}
mkstringify(MultiplicationNode, itoa_(evaluate()));


/* === DivisionNode === */

int DivisionNode::evaluate() {
	return int(lhs->evaluate()) / int(rhs->evaluate());
}
mkstringify(DivisionNode, itoa_(evaluate()));


/* === EnvVarNode === */

EnvVarNode::EnvVarNode(char *name, bool delayedExpansion)
	:super(Node::Type::EnvVar, 0, 0),
	 name(name),
	 delayedExpansion(delayedExpansion)
{
}

char *EnvVarNode::evaluate() {
	if(value == nullptr) return value = getenv(name);
	if(!delayedExpansion) return value;
	return getenv(name);
}

EnvVarNode::operator int() const {
	return atoi(evaluate());
}

mkstringify(EnvVarNode, nullptr);


/* === CallNode === */
CallNode::CallNode(char *name, std::vector<Node*> arguments, bool silent = false)
: Node(Node::Type::Call | WITHCHILDREN, 0, 0),
  silent(silent),
  funcName(strdup(name)),
  hash(_hashfunc_(this->funcName, true))
{
	this->children = new std::vector<Node*>(arguments);
}

int CallNode::evaluate(InterpreterState *state) {
	// todo stringify whole command

	bool dontEcho = this->silent || state->echo;
	if(!dontEcho) {
	}

	if(multicharMapping.count(this->hash) > 0) {
		CallPtr funcPtr = multicharMapping.at(this->hash).second;
		if(funcPtr != nullptr) {
			return funcPtr((CallParams){this->children, state});
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
	: super(Node::Type::Assign, 0, 0)
{
	this->one.lhs = lhs;
	this->one.rhs = rhs;
}

AssignNode::AssignNode(Node **lhs, int lhsCount, Node *rhs)
	: super(Node::Type::Assign | WITHCHILDREN, 0, 0)
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
	this->type = Node::Type::Label;
}

mkstringify(LabelNode, this->str);

/* === CompareNode === */

CompareNode::CompareNode(Node *lhs, Node *rhs, CompareType compareType)
	: super(Node::Type::Compare, 0, 0),
	cmpType(compareType)
{
	this->lhs = lhs;
	this->rhs = rhs;

	switch(compareType) {
	case CompareType::GTR: {
		this->cmpFunc = [](Node *lhs, Node *rhs, bool caseInsensitive) {
			return lhs->num > rhs->num;
		};
		break;
	}
	case CompareType::LSS: {
		this->cmpFunc = [](Node *lhs, Node *rhs, bool caseInsensitive) {
			return lhs->num < rhs->num;
		};
		break;
	}
	case CompareType::GEQ: {
		this->cmpFunc = [](Node *lhs, Node *rhs, bool caseInsensitive) {
			return lhs->num >= rhs->num;
		};
		break;
	}
	case CompareType::LEQ: {
		this->cmpFunc = [](Node *lhs, Node *rhs, bool caseInsensitive) {
			return lhs->num <= rhs->num;
		};
		break;
	}
	case CompareType::EQU: {
		this->cmpFunc = [](Node *lhs, Node *rhs, bool caseInsensitive) {
			return lhs->num == rhs->num;
		};
		break;
	}
	case CompareType::NEQ: {
		this->cmpFunc = [](Node *lhs, Node *rhs, bool caseInsensitive) {
			return lhs->num != rhs->num;
		};
		break;
	}
	case CompareType::STRING: {
		this->cmpFunc = [](Node *lhs, Node *rhs, bool caseInsensitive) {
			//printf("%s %s\n", lhs->str, rhs->str);
			if(caseInsensitive) return strcasecmp(lhs->str, rhs->str) == 0;
			else return strcmp(lhs->str, rhs->str) == 0;
		};
		break;
	}
	case CompareType::EXISTS: {
		this->cmpFunc = [](Node *lhs, Node *rhs, bool caseInsensitive) {
			return access(lhs->str, F_OK) == 0;
		};
		break;
	}
	case CompareType::DEFINED: {
		this->cmpFunc = [](Node *lhs, Node *rhs, bool caseInsensitive) {
			return getenv(lhs->str) != NULL;
		};
		break;
	}
	}
}

bool CompareNode::evaluate(bool caseInsensitive = false) {
	return this->cmpFunc(this->lhs, this->rhs, caseInsensitive);
}

mkstringify(CompareNode, nullptr);

/* == ForNode == */
// params = /type "opts" ()
ForNode::ForNode(ForType type, char id, ParenthesesNode *cond, Node *body, StringNode *opts = nullptr)
	: super(Node::Type::For, 0, 0),
	forType(type),
	id(id),
	loopBody(body),
	opts(opts),
	cond(cond)
{
	// if(params[0]->type == LN(LNODE_SWITCH)) {
	// 	SwitchNode *sw = params[0];
	// 	if(strcasecmp(sw->str, "L") == 0) this->forType = ForType::NUMBERS;
	// 	else if(strcasecmp(sw->str, "R") == 0) this->forType = ForType::FILESROOTED;
	// 	else if(strcasecmp(sw->str, "D")) this->forType = ForType::FOLDERS;
	// 	else if(strcasecmp(sw->str, "F")) {
	// 		ParenthesesNode *pn = params[2];
	// 		if(pn->children[0]->type == LNODE_STRING) {
	// 			StringNode *str = pn->children[0];
	// 			if(str->singlequote) this->forType = ForType::CMDRESULTS;
	// 			else this->forType = ForType::STRINGCONTENTS;
	// 		} else {
	// 			this->forType = ForType::FILECONTENTS;
	// 		}
	// 	}

	// 	this->id = TCAST(IdNode*, params[1])->str[0];
	// } else {
	// 	this->forType = ForType::FILES;
	// 	this->id = TCAST(IdNode*, params[0])->str[0];
	// }

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

uint64_t ForNode::evaluate(InterpreterState *state) {
	for(; this->loopCond(); this->inc()) {
		this->loopBody->evaluate(state);
	}

	return 0;
}

mkstringify(ForNode, nullptr);

IfNode::IfNode(CompareNode *cond, Node *ifBodyRoot, Node *elseBodyRoot = nullptr, bool invert = false, bool caseInsensitive = false)
	: super(Node::Type::If, 0, 0),
	condition(cond),
	sucess(ifBodyRoot),
	failure(elseBodyRoot),
	invert(invert),
	caseInsensitive(caseInsensitive)
{
}

int IfNode::evaluate(InterpreterState *state) {
	if(this->condition->evaluate(this->caseInsensitive) || this->invert) {
		//eval(this->sucess, state);
		this->sucess->evaluate(state);
	} else {
		if(this->failure != nullptr) this->failure->evaluate(state);
	}

	return 0;
}

mkstringify(IfNode, nullptr);
