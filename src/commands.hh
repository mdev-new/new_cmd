#pragma once
#include "nodes.hh"
#include "shared.hh"
#include "lexer.hpp"

extern std::unordered_map<_hashtype_, std::pair<decltype(Token::type), CallPtr>, Hasher> multicharMapping;
void RegisterCommand(char *cmd, CallPtr func);