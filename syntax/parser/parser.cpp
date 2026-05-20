//
// Created by bibib on 2026/3/1.
//

#include "parser.h"

#include <cassert>

#include "../utils.h"
#include "../bs_objs/exception/bs_invalid_syntax_exception.h"
#include "../bs_objs/null/bs_null.h"


parser::parser(std::vector<token> &&tokens, std::string source_code) : tokens_(std::move(tokens)), current_index_(0),
                                                                       source_code_(std::move(source_code)) {
    assert(!tokens_.empty());
    assert(tokens_.back().type == token_type::EOF_);
}

const token &parser::current() const {
    return peek(0);
}

const token &parser::peek(const int offset) const {
    if (current_index_ + offset >= tokens_.size()) return tokens_.back();
    return tokens_[current_index_ + offset];
}

const token &parser::previous() const {
    if (current_index_ == 0) return tokens_[0];
    return tokens_[current_index_ - 1];
}

bool parser::is_at_end() const {
    return current().type == token_type::EOF_;
}

bool parser::match(std::initializer_list<token_type> allow_types, const token &t) {
    const auto it = std::ranges::find(allow_types, t.type);
    return it != allow_types.end();
}

const token &parser::advance() {
    if (!is_at_end()) current_index_++;
    return previous();
}

node_ptr parser::parse_left_assoc(const parse_fn next, const std::initializer_list<token_type> ops) {
    node_ptr left = (this->*next)();
    while (match(ops, current())) {
        const token &t = advance();
        node_ptr right = (this->*next)();
        left = std::make_unique<bin_op_node>(std::move(left), t, std::move(right));
    }

    return left;
}

node_ptr parser::parse() {
    return statements();
}

node_ptr parser::statements() {
    std::vector<node_ptr> nodes;

    while (!is_at_end() && current().type != token_type::DEDENT) {
        while (current().type == token_type::NEWLINE) advance();
        if (is_at_end() || current().type == token_type::DEDENT) break;

        nodes.push_back(statement());

        if (current().type == token_type::NEWLINE)
            advance();
        else if (current().type == token_type::DEDENT || is_at_end() || previous().type == token_type::DEDENT) {
        } else
            throw bs_invalid_syntax_exception{
                "Expected newline after statement, but found '" + current().literal + "'",
                source_code_, current().start,
                current().end
            };
    }

    return std::make_unique<statements_node>(std::move(nodes));
}

node_ptr parser::statement() {
    switch (current().type) {
        case token_type::IF:
            return if_stmt();
        case token_type::WHILE:
            return while_stmt();
        case token_type::UNTIL:
            return until_stmt();
        case token_type::FOR:
            return for_stmt();
        case token_type::FN:
            return fn_stmt();
        case token_type::RETURN:
            return return_stmt();
        case token_type::CONTINUE:
            return continue_stmt();
        case token_type::BREAK:
            return break_stmt();
        default:
            return expr();
    }
}

node_ptr parser::while_stmt() {
    const position start_pos = current().start;
    const bool is_until = current().type == token_type::UNTIL;
    advance();
    node_ptr condition = expr();
    if (current().type != token_type::DO)
        throw bs_invalid_syntax_exception{
            "Expected 'do', but found '" + current().literal + "'", source_code_, current().start, current().end
        };
    advance();
    node_ptr body = block();
    return std::make_unique<while_node>(std::move(condition), std::move(body), is_until,
                                        span{start_pos, body->where.end});
}

node_ptr parser::until_stmt() {
    return while_stmt();
}

node_ptr parser::for_stmt() {
    const position start_pos = advance().start;

    if (current().type != token_type::IDENTIFIER)
        throw bs_invalid_syntax_exception{
            "Expected identifier after 'for'", source_code_, current().start, current().end
        };
    const std::string var_name = current().literal;
    if (peek(1).type != token_type::EQUALS)
        throw bs_invalid_syntax_exception{
            "Expected '=' after for variable", source_code_, current().start, current().end
        };

    node_ptr variable = assignment();
    if (current().type != token_type::TILDE)
        throw bs_invalid_syntax_exception{
            "Expected '~' after for start value", source_code_, current().start, current().end
        };
    advance();
    node_ptr end = expr();
    node_ptr step = nullptr;
    if (current().type == token_type::COLON) {
        advance();
        step = expr();
    }
    if (current().type != token_type::DO)
        throw bs_invalid_syntax_exception{
            "Expected 'do' before 'for' loop body, but found '" + current().literal + "'", source_code_,
            current().start,
            current().end
        };
    advance();
    node_ptr body = block();

    const position end_pos = body->where.end;
    return std::make_unique<for_node>(var_name, std::move(variable), std::move(end), std::move(step), std::move(body),
                                      span{start_pos, end_pos});
}

node_ptr parser::if_stmt() {
    const position start_pos = advance().start;

    node_ptr condition = expr();
    if (current().type != token_type::DO)
        throw bs_invalid_syntax_exception{
            "Expected 'do', but found '" + current().literal + "'", source_code_, current().start, current().end
        };
    advance();
    node_ptr then_branch = block();
    node_ptr else_branch = nullptr;
    while (current().type == token_type::NEWLINE) advance();
    if (current().type == token_type::ELSE) {
        advance();
        if (current().type == token_type::IF)
            else_branch = if_stmt();
        else
            else_branch = block();
    }

    const position end_pos = else_branch ? else_branch->where.end : then_branch->where.end;
    return std::make_unique<if_node>(std::move(condition), std::move(then_branch), std::move(else_branch), span{
                                         start_pos, end_pos
                                     });
}

node_ptr parser::fn_stmt() {
    const position start_pos = advance().start;
    if (current().type != token_type::IDENTIFIER)
        throw bs_invalid_syntax_exception{
            "Expected function name after 'fn', but found '" + current().literal + "'", source_code_, current().start,
            current().end
        };
    const std::string name = advance().literal;

    if (current().type != token_type::LPARN)
        throw bs_invalid_syntax_exception{
            "Expected '(', but found '" + current().literal + "'", source_code_, current().start, current().end
        };
    advance();
    std::vector<std::string> params;
    while (current().type == token_type::IDENTIFIER) {
        params.push_back(advance().literal);
        if (current().type != token_type::COMMA) break;
        advance();
    }

    if (current().type != token_type::RPARN)
        throw bs_invalid_syntax_exception{
            "Expected ')', but found '" + current().literal + "'", source_code_, current().start, current().end
        };
    advance();
    node_ptr body = nullptr;
    if (current().type == token_type::EQUALS) {
        advance();
        body = expr();
    } else if (current().type == token_type::DO) {
        advance();
        body = block();
    } else
        throw bs_invalid_syntax_exception{
            "Expected '=' or 'do', but found '" + current().literal + "'", source_code_, current().start,
            current().end
        };
    const position end_pos = body->where.end;
    return std::make_unique<fn_node>(name, std::move(params), std::move(body), span{start_pos, end_pos});
}

node_ptr parser::return_stmt() {
    const position start_pos = current().start;
    position end_pos = advance().end;
    if (current().type == token_type::NEWLINE)
        return std::make_unique<return_node>(nullptr, span{start_pos, end_pos});
    node_ptr return_val = expr();
    end_pos = return_val->where.end;
    return std::make_unique<return_node>(std::move(return_val), span{start_pos, end_pos});
}

node_ptr parser::break_stmt() {
    const position start_pos = current().start;
    const position end_pos = advance().end;
    return std::make_unique<break_node>(span{start_pos, end_pos});
}

node_ptr parser::continue_stmt() {
    const position start_pos = current().start;
    const position end_pos = advance().end;
    return std::make_unique<continue_node>(span{start_pos, end_pos});
}

node_ptr parser::block() {
    if (current().type == token_type::NEWLINE) {
        while (current().type == token_type::NEWLINE) advance();
        if (current().type != token_type::INDENT)
            throw bs_invalid_syntax_exception{
                "Expected indentation after newline", source_code_, current().start,
                current().end
            };
        advance();
        node_ptr res = statements();
        if (current().type != token_type::DEDENT && !is_at_end())
            throw bs_invalid_syntax_exception{
                "Expected dedent at the end of the block", source_code_, current().start,
                current().end
            };
        if (current().type == token_type::DEDENT) advance();
        return res;
    }
    return statement();
}

node_ptr parser::expr() {
    return assignment();
}

node_ptr parser::assignment() {
    if (current().type == token_type::IDENTIFIER && match({
                                                              token_type::EQUALS, token_type::PLUS_EQ,
                                                              token_type::SUB_EQ, token_type::MUL_EQ,
                                                              token_type::DIV_EQ, token_type::MOD_EQ, token_type::POW_EQ
                                                          }, peek(1))) {
        const token &var_name = advance();
        token_type assign_type = advance().type;
        node_ptr value = expr();
        return std::make_unique<var_assign_node>(var_name, assign_type, std::move(value));
    }

    return logic_or();
}

node_ptr parser::logic_or() {
    return parse_left_assoc(&parser::logic_xor, {token_type::OR});
}

node_ptr parser::logic_xor() {
    return parse_left_assoc(&parser::logic_and, {token_type::XOR});
}

node_ptr parser::logic_and() {
    return parse_left_assoc(&parser::comparison, {token_type::AND});
}

node_ptr parser::comparison() {
    node_ptr left = sum();

    if (!match({token_type::LESS, token_type::GREATER, token_type::LE, token_type::GE, token_type::EQ, token_type::NEQ},
               current()))
        return left;


    std::vector<node_ptr> operands;
    std::vector<token_type> operators;

    operands.push_back(std::move(left));

    while (match({
                     token_type::LESS, token_type::GREATER, token_type::LE, token_type::GE, token_type::EQ,
                     token_type::NEQ
                 },
                 current())) {
        operators.push_back(advance().type);
        operands.push_back(sum());
    }

    return std::make_unique<comparison_node>(std::move(operands), std::move(operators));
}

node_ptr parser::sum() {
    return parse_left_assoc(&parser::term, {token_type::PLUS, token_type::SUBTRACT});
}

node_ptr parser::term() {
    return parse_left_assoc(&parser::power, {token_type::MULTIPLY, token_type::DIVISION, token_type::MODULO});
}

node_ptr parser::power() {
    node_ptr left = unary();
    if (current().type == token_type::POWER) {
        const token &t = advance();
        node_ptr right = power();
        return std::make_unique<bin_op_node>(std::move(left), t, std::move(right));
    }

    return left;
}

node_ptr parser::unary() {
    if (match({token_type::PLUS, token_type::SUBTRACT, token_type::NOT}, current())) {
        const token &t = advance();
        node_ptr operand = unary();
        return std::make_unique<unary_op_node>(t, std::move(operand));
    }
    return postfix();
}

node_ptr parser::postfix() {
    node_ptr number = call();
    while (current().type == token_type::FACTORIAL) {
        const token &t = advance();
        number = std::make_unique<factorial_node>(std::move(number), t);
    }
    return number;
}

node_ptr parser::call() {
    if (current().type != token_type::IDENTIFIER || peek(1).type != token_type::LPARN)
        return primary();
    const position start_pos = current().start;
    const std::string name = advance().literal;
    advance();
    std::vector<node_ptr> args;
    while (current().type != token_type::RPARN) {
        args.push_back(expr());
        if (current().type != token_type::COMMA) break;
        advance();
    }
    if (current().type != token_type::RPARN)
        throw bs_invalid_syntax_exception{
            "Expected ')', but found '" + current().literal + "'", source_code_, current().start, current().end
        };
    const position end_pos = advance().end;
    return std::make_unique<call_node>(name, std::move(args), span{start_pos, end_pos});
}

node_ptr parser::primary() {
    if (current().type == token_type::NUMBER) {
        const token &t = advance();
        return std::make_unique<number_node>(t);
    }
    if (current().type == token_type::LPARN) {
        const position start_pos = current().start;
        advance();
        node_ptr expression = expr();
        if (current().type != token_type::RPARN)
            throw bs_invalid_syntax_exception{
                "Expected ')' but found '" + current().literal + "'",
                source_code_, current().start,
                current().end
            };
        const position end_pos = current().end;
        advance();
        expression->where = {start_pos, end_pos};
        return expression;
    }
    if (current().type == token_type::IDENTIFIER) {
        const token &t = advance();
        return std::make_unique<var_access_node>(t);
    }

    if (match({token_type::TRUE, token_type::FALSE}, current())) {
        const token &t = advance();
        return std::make_unique<boolean_node>(t);
    }

    if (current().type == token_type::NULL_LIT) {
        const token &t = advance();
        return std::make_unique<none_node>(t);
    }

    if (current().type == token_type::STRING) {
        const token &t = advance();
        return std::make_unique<string_node>(t);
    }

    throw bs_invalid_syntax_exception{
        "Expected '(', identifier, or literal, but found '" + current().literal + "'",
        source_code_, current().start,
        current().end
    };
}
