//
// Created by bibib on 2026/2/20.
//

#ifndef BSHARP_LEXER_H
#define BSHARP_LEXER_H
#include <stack>
#include <string>
#include <vector>

#include "../token/token.h"


class lexer {
public:
    explicit lexer(std::string input, std::string &&file_name = "<stdin>");

    std::vector<token> tokenize();

private:
    std::string input_;
    std::string file_name_;
    size_t index_;
    position position_;
    char current_char_;
    std::stack<int> indent_stack_;
    bool is_line_start_;

    void advance();

    std::string make_number();

    std::string make_identifier();

    std::string make_operator();

    std::string make_string();
};


#endif //BSHARP_LEXER_H
