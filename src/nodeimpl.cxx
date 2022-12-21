#pragma once
#include "parser.hpp"

// like interpreter, this file HAS to be optimized aswell

int evalExpr(Node *root) {
	switch(root->type) {
	case MKNTYP(NODE_LEAF, LNODE_NUMBER): return ((NumberNode*)root)->evaluate();
	case MKNTYP(NODE_INNER, INODE_PARENTHESES): return ((ParenthesesNode*)root)->evaluate();
	case MKNTYP(NODE_INNER, INODE_BINOP): return ((NumberNode*)((BinOpNode*)root)->evaluate())->evaluate();
	}
}

/* === NumberNode === */

NumberNode::NumberNode(int n) {
	this->num = n;
}

int NumberNode::evaluate() { return num; }


/* === StringNode === */

StringNode::StringNode(const char *s) {
	this->str = s;
	this->type = MKNTYP(NODE_LEAF, LNODE_STRING);
}

const char *StringNode::evaluate() { return str; }


/* === ParenthesesNode === */

ParenthesesNode::ParenthesesNode() {
	this->children = malloc(threshold*sizeof(Node*));
	this->childrenCount = 0;
	this->type = MKNTYP(NODE_INNER, INODE_PARENTHESES);
}

void ParenthesesNode::append(Node *n) {
	if(this->childrenCount+1 >= this->threshold) this->children = realloc(this->children, (threshold += 16)*sizeof(Node*));
	children[childrenCount++] = n;

	//printf("APPEND: %d %d\n", childrenCount, n->type);
}

int ParenthesesNode::evaluate() {
	// todo implement the algorithm using skip things etc
	// MUST BE OPTIMIZED

	printf("!!! UNIMPLEMENTED !!!: %s", __PRETTY_FUNCTION__);
	
	return -1;
}


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


/* === SubtractionNode === */

int SubtractionNode::evaluate() {
	return evalExpr(lhs) - evalExpr(rhs);
}


/* === MultiplicationNode === */

int MultiplicationNode::evaluate() {
	return evalExpr(lhs) * evalExpr(rhs);
}


/* === DivisionNode */

int DivisionNode::evaluate() {
	return evalExpr(lhs) / evalExpr(rhs);
}


/* === EnvVarNode === */

EnvVarNode::EnvVarNode(const char *name) : name(name) {}

void EnvVarNode::init() {
	value = getenv(name);
}

const char *EnvVarNode::evaluate(bool delayedExpansion) {
	if(!delayedExpansion) return value;
	return value = getenv(name);
}
