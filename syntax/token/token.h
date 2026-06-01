//
// Created by bibibird on 2026/2/20.
//

#ifndef BSHARP_TOKEN_H
#define BSHARP_TOKEN_H
#include <string>

#include "token_types.h"
#include "../position/position.h"


struct token {
    const std::string literal;
    const token_type type;
    const position start;
    const position end;
};


#endif //BSHARP_TOKEN_H