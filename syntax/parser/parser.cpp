//
// Created by bibib on 2026/3/1.
//

#include "parser.h"

#include <cassert>

#include "../utils.h"
#include "../bs_objs/exception/bs_invalid_syntax_exception.h"
#include "../bs_objs/null/bs_null.h"

namespace {
    std::string describe_token(const token &t) {
        switch (t.type) {
            case token_type::EOF_:
                return "<end of file>";
            case token_type::NEWLINE:
                return "<newline>";
            case token_type::INDENT:
                return "<indent>";
            case token_type::DEDENT:
                return "<dedent>";
            default:
                if (t.literal.empty()) return "<empty token>";
                return "'" + t.literal + "'";
        }
    }

    std::string expected_message(const std::string &expected, const token &found) {
        return "expected " + expected + ", but found " + describe_token(found);
    }
}


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
                expected_message("newline after statement", current()),
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
            expected_message("'do'", current()), source_code_, current().start, current().end
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
            expected_message("identifier after 'for'", current()), source_code_, current().start, current().end
        };
    const std::string var_name = current().literal;
    if (peek(1).type != token_type::EQUALS)
        throw bs_invalid_syntax_exception{
            expected_message("'=' after for loop variable", peek(1)), source_code_, peek(1).start, peek(1).end
        };

    node_ptr variable = assignment();
    if (current().type != token_type::TILDE)
        throw bs_invalid_syntax_exception{
            expected_message("'~' after for start value", current()), source_code_, current().start, current().end
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
            expected_message("'do' before 'for' loop body", current()), source_code_,
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
            expected_message("'do'", current()), source_code_, current().start, current().end
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
    return std::make_unique<if_node>(std::move(condition), std::move(then_branch), std::move(else_branch),
                                     span{start_pos, end_pos}, true);
}

node_ptr parser::fn_stmt() {
    const position start_pos = advance().start;
    if (current().type != token_type::IDENTIFIER)
        throw bs_invalid_syntax_exception{
            expected_message("function name after 'fn'", current()), source_code_, current().start,
            current().end
        };
    const std::string name = advance().literal;

    if (current().type != token_type::LPARN)
        throw bs_invalid_syntax_exception{
            expected_message("'('", current()), source_code_, current().start, current().end
        };
    advance();
    std::vector<parameter> params;

    if (current().type != token_type::RPARN)
        for (;;) {
            if (current().type != token_type::IDENTIFIER)
                throw bs_invalid_syntax_exception{
                    expected_message("parameter name", current()), source_code_, current().start, current().end
                };
            const std::string var_name = advance().literal;
            bool is_variadic = false;
            if (current().type == token_type::ELLIPSIS) {
                is_variadic = true;
                advance();
                if (const token_type next_type = current().type;
                    (next_type != token_type::COMMA || peek(1).type != token_type::RPARN) && next_type !=
                    token_type::RPARN)
                    throw bs_invalid_syntax_exception{
                        expected_message("')' after variadic parameter", current()),
                        source_code_, current().start, current().end
                    };
            }
            params.push_back(parameter{var_name, is_variadic});
            if (current().type != token_type::COMMA) break;
            advance();
            if (current().type == token_type::RPARN) break;
        }

    if (current().type != token_type::RPARN)
        throw bs_invalid_syntax_exception{
            expected_message("')'", current()), source_code_, current().start, current().end
        };
    advance();
    node_ptr body = nullptr;
    if (current().type == token_type::EQUALS) {
        advance();
        const position expr_start = current().start;
        node_ptr expression = expr();
        const position expr_end = expression->where.end;
        body = std::make_unique<return_node>(std::move(expression), span{expr_start, expr_end});
    } else if (current().type == token_type::DO) {
        advance();
        body = block();
    } else
        throw bs_invalid_syntax_exception{
            expected_message("'=' or 'do'", current()), source_code_, current().start,
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
                expected_message("indentation after newline", current()), source_code_, current().start,
                current().end
            };
        advance();
        node_ptr res = statements();
        if (current().type != token_type::DEDENT && !is_at_end())
            throw bs_invalid_syntax_exception{
                expected_message("dedent at the end of the block", current()), source_code_, current().start,
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
    node_ptr left = logic_or();
    if (match({
                  token_type::EQUALS, token_type::PLUS_EQ, token_type::SUB_EQ, token_type::MUL_EQ, token_type::DIV_EQ,
                  token_type::MOD_EQ, token_type::POW_EQ
              }, current())) {
        const token op = advance();
        node_ptr value = expr();

        if (const auto var_node = dynamic_cast<const var_access_node *>(left.get()))
            return std::make_unique<var_assign_node>(token{
                                                         var_node->name, token_type::IDENTIFIER, var_node->where.start,
                                                         var_node->where.end
                                                     }, op.type, std::move(value));


        if (const auto sub_node = dynamic_cast<subscript_node *>(left.get())) {
            const position start_pos = sub_node->where.start;
            const position end_pos = value->where.end;

            const auto concrete_sub_node = std::unique_ptr<subscript_node>(
                dynamic_cast<subscript_node *>(left.release()));

            return std::make_unique<subscript_assign_node>(
                std::move(concrete_sub_node->left),
                std::move(concrete_sub_node->index),
                op.type,
                std::move(value),
                span{start_pos, end_pos}
            );
        }

        throw bs_invalid_syntax_exception{"invalid assignment target", source_code_, op.start, op.end};
    }
    return left;
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

    if (!match({
                   token_type::LESS, token_type::GREATER, token_type::LE, token_type::GE, token_type::EQ,
                   token_type::NEQ
               },
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
    node_ptr expr_node = primary();
    const position start_pos = expr_node->where.start;
    position end_pos = expr_node->where.end;

    for (;;) {
        if (current().type == token_type::LPARN) {
            advance();
            std::vector<node_ptr> args;
            if (current().type != token_type::RPARN)
                for (;;) {
                    args.push_back(expr());
                    if (current().type != token_type::COMMA) break;
                    advance();
                    if (current().type == token_type::RPARN) break;
                }

            if (current().type != token_type::RPARN)
                throw bs_invalid_syntax_exception{
                    expected_message("')'", current()), source_code_, current().start,
                    current().end
                };
            end_pos = advance().end;

            expr_node = std::make_unique<
                call_node>(std::move(expr_node), std::move(args), span{start_pos, end_pos});
        } else if (current().type == token_type::LBRACKET) {
            advance();
            node_ptr index = expr();
            if (current().type != token_type::RBRACKET)
                throw bs_invalid_syntax_exception{
                    expected_message("']'", current()), source_code_, current().start,
                    current().end
                };
            end_pos = advance().end;

            expr_node = std::make_unique<subscript_node>(std::move(expr_node), std::move(index),
                                                         span{start_pos, end_pos});
        } else if (current().type == token_type::FACTORIAL) {
            const token &t = advance();
            expr_node = std::make_unique<factorial_node>(std::move(expr_node), t);
        } else break;
    }
    return expr_node;
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
                expected_message("')'", current()),
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
        return std::make_unique<null_node>(t);
    }

    if (current().type == token_type::STRING) {
        const token &t = advance();
        return std::make_unique<string_node>(t);
    }

    if (current().type == token_type::LBRACKET)
        return list_literal();

    if (current().type == token_type::IF)
        return ternary();


    throw bs_invalid_syntax_exception{
        expected_message("an expression", current()),
        source_code_, current().start,
        current().end
    };
}

node_ptr parser::list_literal() {
    const position start_pos = advance().start;
    std::vector<node_ptr> args;

    if (current().type != token_type::RBRACKET)
        for (;;) {
            args.push_back(expr());
            if (current().type != token_type::COMMA) break;
            advance();
            if (current().type == token_type::RBRACKET) break;
        }

    if (current().type != token_type::RBRACKET)
        throw bs_invalid_syntax_exception{
            expected_message("']'", current()), source_code_, current().start, current().end
        };
    const position end_pos = advance().end;
    return std::make_unique<list_literal_node>(std::move(args), span{start_pos, end_pos});
}

node_ptr parser::ternary() {
    const position start_pos = advance().start;
    node_ptr condition = expr();
    if (current().type != token_type::DO)
        throw bs_invalid_syntax_exception{
            expected_message("'do'", current()), source_code_, current().start, current().end
        };

    advance();

    node_ptr then_branch = expr();
    if (current().type != token_type::ELSE)
        throw bs_invalid_syntax_exception{
            expected_message("'else' in if expression", current()),
            source_code_, current().start, current().end
        };
    advance();

    node_ptr false_branch = expr();
    const position end_pos = false_branch->where.end;

    return std::make_unique<if_node>(std::move(condition), std::move(then_branch), std::move(false_branch),
                                     span{start_pos, end_pos}, false);
}
