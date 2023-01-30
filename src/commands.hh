#pragma once
#include "nodes.hh"
#include "shared.hh"
#include "lexer.hpp"

#define fe(x,y,z) {consthash(x), { y,z } },

extern std::unordered_map<_hashtype_, std::pair<decltype(Token::type), CallPtr>, Hasher> multicharMapping;
void RegisterCommand(char *cmd, CallPtr func);