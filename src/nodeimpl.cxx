#pragma once
#include "parser.hpp"
#include <cstdlib>
#include <cstdio>

// like interpreter, this file HAS to be optimized aswell

char *itoa_(int i) {
	static char b[21] = {0};
	char *c=b+20;
	int x=abs(i);
	do  *--c = 48 + x % 10;
	while(x && (x/=10));
	if(i<0) *--c = 45;
	return c;
}

int evalExpr(Node *root) {
	switch(root->type) {
	case MKNTYP(NODE_LEAF, LNODE_NUMBER) & BARETYPE: return ((NumberNode*)root)->evaluate();
	case MKNTYP(NODE_INNER, INODE_PARENTHESES) & BARETYPE: return ((ParenthesesNode*)root)->evaluate();
	case MKNTYP(NODE_INNER, INODE_BINOP) & BARETYPE: return ((NumberNode*)((BinOpNode*)root)->evaluate())->evaluate();
	//default: return -1;
	}
}

/* === NumberNode === */

NumberNode::NumberNode(int n) {
	this->num = n;
	this->type = MKNTYP(NODE_LEAF, LNODE_NUMBER);
}

int NumberNode::evaluate() { return num; }
std::pair<const char *, char *> NumberNode::stringify() { return std::make_pair("NumberNode", itoa_(this->num)); };

/* === StringNode === */

StringNode::StringNode(char *s) {
	this->str = strdup(s);
	this->type = MKNTYP(NODE_LEAF, LNODE_STRING);
}

const char *StringNode::evaluate() { return str; }
std::pair<const char *, char *> StringNode::stringify() { return std::make_pair("StringNode", this->str); };


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
std::pair<const char *, char *> ParenthesesNode::stringify() { return std::make_pair("ParenthesesNode", nullptr); };


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
std::pair<const char *, char *> AdditionNode::stringify() { return std::make_pair("AdditionNode", nullptr); };


/* === SubtractionNode === */

int SubtractionNode::evaluate() {
	return evalExpr(lhs) - evalExpr(rhs);
}
std::pair<const char *, char *> SubtractionNode::stringify() { return std::make_pair("SubtractionNode", nullptr); };


/* === MultiplicationNode === */

int MultiplicationNode::evaluate() {
	return evalExpr(lhs) * evalExpr(rhs);
}
std::pair<const char *, char *> MultiplicationNode::stringify() { return std::make_pair("MultiplicationNode", nullptr); };


/* === DivisionNode === */

int DivisionNode::evaluate() {
	return evalExpr(lhs) / evalExpr(rhs);
}
std::pair<const char *, char *> DivisionNode::stringify() { return std::make_pair("DivisionNode", nullptr); };


/* === EnvVarNode === */

EnvVarNode::EnvVarNode(char *name) : name(name) {
	this->type = MKNTYP(NODE_LEAF, LNODE_ENVVAR);
}

void EnvVarNode::init() {
	value = getenv(name);
}

char *EnvVarNode::evaluate(bool delayedExpansion) {
	if(!delayedExpansion) return value;
	return getenv(name);
}
std::pair<const char *, char *> EnvVarNode::stringify() { return std::make_pair("EnvVarNode", nullptr); };


/* === CallNode === */
CallNode::CallNode(char *name, std::vector<Node*> args)
:  args(args)
{
	this->funcName = strdup(name);
	this->type = MKNTYP(NODE_LEAF, LNODE_CALL);
	printf("%d %d\n", this->type, MKNTYP(NODE_LEAF, LNODE_CALL));
}

int CallNode::execute() {
	size_t hashed = hash(this->funcName);

	if(multicharMapping.count(hashed) > 0) {
		CallPtr funcPtr = multicharMapping.at(hashed).second;
		if(funcPtr != nullptr) {
			funcPtr((CallParams){args.data(), args.size()});
		}
	} else {
		// todo find the executable
		// and execute with stringified params
	}
}
std::pair<const char *, char *> CallNode::stringify() { return std::make_pair("CallNode", this->funcName); };


/* === EqualsNode === */
AssignNode::AssignNode(Node *lhs, Node *rhs) {
	this->_1x1.lhs = lhs;
	this->_1x1.rhs = rhs;
	this->type = MKNTYP(NODE_INNER, INODE_ASSIGN);
}

AssignNode::AssignNode(Node **lhs, int lhsCount, Node *rhs) {
	this->_Xx1.lhs = lhs;
	this->_Xx1.rhs = rhs;
	this->_Xx1.lhsCount = lhsCount;
	this->type = MKNTYP(NODE_INNER, INODE_ASSIGN) | WITHCHILDREN;
}
std::pair<const char *, char *> AssignNode::stringify() { return std::make_pair("AssignNode", nullptr); };


/* === LabelNode === */

LabelNode::LabelNode(int pos)
 : filePos(pos)
{
	this->type = MKNTYP(NODE_LEAF, LNODE_LABEL);
}
std::pair<const char *, char *> LabelNode::stringify() { return std::make_pair("LabelNode", nullptr); };
