#pragma once
#include "parser.hpp"
#include "interpreter.hh"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "win.hh"

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

int evalExpr(Node *root) {
	switch(root->type & BARETYPE) {
	case MKNTYP(NODE_LEAF, LNODE_NUMBER): return ((NumberNode*)root)->evaluate();
	case MKNTYP(NODE_INNER, INODE_PARENTHESES): return ((ParenthesesNode*)root)->evaluate();
	case MKNTYP(NODE_INNER, INODE_BINOP): return ((NumberNode*)((BinOpNode*)root)->evaluate())->evaluate();
	//default: return -1;
	}
}

/* === NumberNode === */

NumberNode::NumberNode(int n) {
	this->num = n;
	this->type = MKNTYP(NODE_LEAF, LNODE_NUMBER);
}

int NumberNode::evaluate() { return num; }
mkstringify(NumberNode, itoa_(this->num));

/* === StringNode === */

StringNode::StringNode(char *s) {
	this->str = strdup(s);
	this->type = MKNTYP(NODE_LEAF, LNODE_STRING);
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

ParenthesesNode::ParenthesesNode() {
	this->children = malloc(threshold*sizeof(Node*));
	this->childrenCount = 0;
	this->type = MKNTYP(NODE_INNER, INODE_PARENTHESES) | WITHCHILDREN;
}

void ParenthesesNode::append(Node *n) {
	if(this->childrenCount+1 >= this->threshold) this->children = realloc(this->children, (threshold += 16)*sizeof(Node*));
	children[childrenCount++] = n;

	//printf("APPEND: %d %d\n", childrenCount, n->type);
}

int ParenthesesNode::evaluate() {
	// we'll probably be evaluating commands here aswell

	printf("!!! UNIMPLEMENTED !!!: %s", __PRETTY_FUNCTION__);

	return -1;
}
mkstringify(ParenthesesNode, nullptr);


/* === BinOpNode === */

BinOpNode::BinOpNode(Node *lhs, Node *rhs) {
	this->lhs = lhs;
	this->rhs = rhs;
	this->type = MKNTYP(NODE_INNER, INODE_BINOP);
}


/* === AdditionNode === */

int AdditionNode::evaluate() {
	return evalExpr(lhs) + evalExpr(rhs);
}
mkstringify(AdditionNode, itoa_(evaluate()));


/* === SubtractionNode === */

int SubtractionNode::evaluate() {
	return evalExpr(lhs) - evalExpr(rhs);
}
mkstringify(SubtractionNode, itoa_(evaluate()));


/* === MultiplicationNode === */

int MultiplicationNode::evaluate() {
	return evalExpr(lhs) * evalExpr(rhs);
}
mkstringify(MultiplicationNode, itoa_(evaluate()));


/* === DivisionNode === */

int DivisionNode::evaluate() {
	return evalExpr(lhs) / evalExpr(rhs);
}
mkstringify(DivisionNode, itoa_(evaluate()));


/* === EnvVarNode === */

EnvVarNode::EnvVarNode(char *name) : name(name) {
	this->type = MKNTYP(NODE_LEAF, LNODE_ENVVAR);
}

char *EnvVarNode::evaluate(bool delayedExpansion) {
	if(value == nullptr) return value = getenv(name);
	if(!delayedExpansion) return value;
	return getenv(name);
}
mkstringify(EnvVarNode, nullptr);


/* === CallNode === */
CallNode::CallNode(char *name, std::vector<Node*> arguments, bool silent = false)
:  args(arguments),
   silent(silent)
{
	this->funcName = strdup(name);
	this->type = MKNTYP(NODE_LEAF, LNODE_CALL);
}

int CallNode::execute(InterpreterState *state) {
	_hashtype_ hashed = _hashfunc_(this->funcName);

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
	}

	return 0;
}
mkstringify(CallNode, this->funcName);


/* === EqualsNode === */
AssignNode::AssignNode(Node *lhs, Node *rhs) {
	this->one.lhs = lhs;
	this->one.rhs = rhs;
	this->type = MKNTYP(NODE_INNER, INODE_ASSIGN);
}

AssignNode::AssignNode(Node **lhs, int lhsCount, Node *rhs) {
	this->many.lhs = lhs;
	this->many.rhs = rhs;
	this->many.lhsCount = lhsCount;
	this->type = MKNTYP(NODE_INNER, INODE_ASSIGN) | WITHCHILDREN;
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
