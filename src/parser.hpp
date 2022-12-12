#pragma once
#include "lexer.hpp"
#include <cstring>
#include <vector>
#include <memory>

// original thought was to compile the whole program
// into series of nodes that would get evaluated at runtime

// I hate that this has to be here but whatever
enum NodeType {
	NODE_LEAF,
	NODE_INNER
};

enum InnerNodeType {
	INODE_PARENTHESES,
	INODE_ADD,
	INODE_SUB,
	INODE_MUL,
	INODE_DIV
};

enum LeafNodeType {
};

struct Node {
	uint16_t type; // .......yyyyyyyyx => x = leaf/inner, y = leaf/inner type
};

struct LeafNode : public Node {
};

struct InnerNode : public Node {
	std::vector<std::unique_ptr<Node>> children;
};

struct BinOpNode : public InnerNode {};

struct AdditionNode : public BinOpNode {};
struct SubtractionNode : public BinOpNode {};
struct MultiplicationNode : public BinOpNode {};
struct DivisionNode : public BinOpNode {};
struct EnvVarNode {};

struct ParsedFile {
	Node** nodes;
	int ncount;
};

ParsedFile parse(LexedFile &lexedFile);