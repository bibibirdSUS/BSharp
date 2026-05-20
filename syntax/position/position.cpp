//
// Created by bibib on 2026/2/20.
//

#include "position.h"

void position::advance(const char curr) {
    if (curr == '\n') {
        line++;
        column = 0;
    } else column++;
}
