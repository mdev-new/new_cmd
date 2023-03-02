#pragma once
#include "interpreter/nodes.hh"
#include "interpreter/lexer/token.hpp"
#include "utilities/hashing.hpp"

#define fe(x,y,z) {consthash(x), { y,z } },

extern std::unordered_map<_hashtype_, std::pair<decltype(Token::type), CallPtr>, Hasher> multicharMapping;