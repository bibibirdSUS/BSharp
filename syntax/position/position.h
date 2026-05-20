//
// Created by bibib on 2026/2/20.
//

#ifndef BSHARP_POSITION_H
#define BSHARP_POSITION_H
#include <string>


struct position {
    std::string file_name;
    size_t line;
    size_t column;

    void advance(char curr);
};


#endif //BSHARP_POSITION_H
