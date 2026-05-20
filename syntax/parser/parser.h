//
// Created by bibib on 2026/3/1.
//

#ifndef BSHARP_PARSER_H
#define BSHARP_PARSER_H
#include <vector>

#include "nodes.h"
#include "../token/token.h"


class parser {
    typedef node_ptr (parser::*parse_fn)();

public:
    parser(std::vector<token> &&tokens, std::string source_code);

    node_ptr parse();

private:
    std::vector<token> tokens_;
    size_t current_index_;
    std::string source_code_;

    [[nodiscard]] const token &current() const;

    [[nodiscard]] const token &peek(int offset) const;

    [[nodiscard]] const token &previous() const;

    [[nodiscard]] bool is_at_end() const;

    const token &advance();

    static bool match(std::initializer_list<token_type> allow_types, const token &t);

    node_ptr statements();

    node_ptr statement();

    node_ptr if_stmt();

    node_ptr while_stmt();

    node_ptr until_stmt();

    node_ptr for_stmt();

    node_ptr fn_stmt();

    node_ptr return_stmt();

    node_ptr break_stmt();

    node_ptr continue_stmt();

    node_ptr block();

    node_ptr expr();

    node_ptr assignment();

    node_ptr logic_or();

    node_ptr logic_xor();

    node_ptr logic_and();

    node_ptr comparison();

    node_ptr sum();

    node_ptr term();

    node_ptr power();

    node_ptr unary();

    node_ptr postfix();

    node_ptr call();

    node_ptr primary();

    node_ptr parse_left_assoc(parse_fn next, std::initializer_list<token_type> ops);
};


#endif //BSHARP_PARSER_H
