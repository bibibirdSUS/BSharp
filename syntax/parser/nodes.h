//
// Created by bibib on 2026/3/1.
//

#ifndef BSHARP_NODES_H
#define BSHARP_NODES_H
#include <memory>
#include <utility>
#include <vector>


#include "../position/position.h"
#include "../token/token_types.h"
#include "../token/token.h"


struct span {
    position start;
    position end;
};

class interpreter;
class bs_obj;
class context;
typedef std::shared_ptr<context> context_ptr;
struct node;
typedef std::shared_ptr<bs_obj> bs_obj_ptr;
typedef std::unique_ptr<node> node_ptr;

struct node {
    span where;

    explicit node(span &&where) : where(std::move(where)) {
    }

    virtual ~node() = default;

    virtual bs_obj_ptr accept(interpreter &visitor, context_ptr ctx) = 0;

    [[nodiscard]] virtual node_ptr copy() const = 0;
};


struct number_node final : node {
    double value;

    explicit number_node(const token &t) : node(span{t.start, t.end}),
                                           value(std::stod(t.literal)) {
    }

    bs_obj_ptr accept(interpreter &visitor, context_ptr ctx) override;

    [[nodiscard]] node_ptr copy() const override;
};

struct boolean_node final : node {
    bool value;

    explicit boolean_node(const token &t) : node(span{t.start, t.end}),
                                            value(t.type == token_type::TRUE) {
    }

    bs_obj_ptr accept(interpreter &visitor, context_ptr ctx) override;

    [[nodiscard]] node_ptr copy() const override;
};

struct none_node final : node {
    explicit none_node(const token &t) : node(span{t.start, t.end}) {
    }

    bs_obj_ptr accept(interpreter &visitor, context_ptr ctx) override;

    [[nodiscard]] node_ptr copy() const override;
};

struct comparison_node final : node {
    std::vector<node_ptr> operands;
    std::vector<token_type> operators;

    comparison_node(std::vector<node_ptr> operands, std::vector<token_type> operators)
        : node(span{operands.front()->where.start, operands.back()->where.end}),
          operands(std::move(operands)), operators(std::move(operators)) {
    }

    bs_obj_ptr accept(interpreter &visitor, context_ptr ctx) override;

    [[nodiscard]] node_ptr copy() const override;
};

struct bin_op_node final : node {
    node_ptr left;
    node_ptr right;
    token_type op;

    bin_op_node(node_ptr left, const token &t, node_ptr right) : node(span{left->where.start, right->where.end}),
                                                                 left(std::move(left)),
                                                                 right(std::move(right)), op(t.type) {
    }

    bs_obj_ptr accept(interpreter &visitor, context_ptr ctx) override;

    [[nodiscard]] node_ptr copy() const override;
};

struct unary_op_node final : node {
    node_ptr operand;
    token_type op;

    unary_op_node(const token &t, node_ptr operand) : node(span{t.start, operand->where.end}),
                                                      operand(std::move(operand)),
                                                      op(t.type) {
    }

    bs_obj_ptr accept(interpreter &visitor, context_ptr ctx) override;

    [[nodiscard]] node_ptr copy() const override;
};

struct factorial_node final : node {
    node_ptr operand;

    factorial_node(node_ptr operand, const token &t) : node(span{operand->where.start, t.end}),
                                                       operand(std::move(operand)) {
    }

    bs_obj_ptr accept(interpreter &visitor, context_ptr ctx) override;

    [[nodiscard]] node_ptr copy() const override;
};

struct var_assign_node final : node {
    std::string name;
    node_ptr value;
    token_type op;

    explicit var_assign_node(const token &var, const token_type op, node_ptr value) : node(span{
            var.start, value->where.end
        }),
        name(var.literal), value(std::move(value)), op(op) {
    }

    bs_obj_ptr accept(interpreter &visitor, context_ptr ctx) override;

    [[nodiscard]] node_ptr copy() const override;
};

struct var_access_node final : node {
    std::string name;

    explicit var_access_node(const token &var) : node(span{var.start, var.end}), name(var.literal) {
    }

    bs_obj_ptr accept(interpreter &visitor, context_ptr ctx) override;

    [[nodiscard]] node_ptr copy() const override;
};

struct statements_node final : node {
    std::vector<node_ptr> statements;

    explicit statements_node(std::vector<node_ptr> &&statements)
        : node(statements.empty()
                   ? span{position{"", 0, 0}, position{"", 0, 0}}
                   : span{
                       statements.front()->where.start,
                       statements.back()->where.end
                   }),
          statements(std::move(statements)) {
    }

    bs_obj_ptr accept(interpreter &visitor, context_ptr ctx) override;

    [[nodiscard]] node_ptr copy() const override;
};

struct if_node final : node {
    node_ptr condition;
    node_ptr then_branch;
    node_ptr else_branch;

    if_node(node_ptr condition, node_ptr then_branch, node_ptr else_branch, span where)
        : node(std::move(where)), condition(std::move(condition)),
          then_branch(std::move(then_branch)), else_branch(std::move(else_branch)) {
    }

    bs_obj_ptr accept(interpreter &visitor, context_ptr ctx) override;

    [[nodiscard]] node_ptr copy() const override;
};

struct while_node final : node {
    node_ptr condition;
    node_ptr body;
    bool is_until;

    while_node(node_ptr condition, node_ptr body, const bool is_until, span where) : node(std::move(where)),
        condition(std::move(condition)),
        body(std::move(body)), is_until(is_until) {
    }

    bs_obj_ptr accept(interpreter &visitor, context_ptr ctx) override;

    [[nodiscard]] node_ptr copy() const override;
};

struct for_node final : node {
    std::string name;
    node_ptr start;
    node_ptr end;
    node_ptr step;
    node_ptr body;

    for_node(std::string name, node_ptr start, node_ptr end, node_ptr step, node_ptr body, span where) : node(
            std::move(where)), name(std::move(name)), start(std::move(start)), end(std::move(end)),
        step(std::move(step)), body(std::move(body)) {
    }

    bs_obj_ptr accept(interpreter &visitor, context_ptr ctx) override;

    [[nodiscard]] node_ptr copy() const override;
};

struct fn_node final : node {
    std::string name;
    std::vector<std::string> params;
    node_ptr body;

    fn_node(std::string name, std::vector<std::string> params, node_ptr body, span where) : node(std::move(where)),
        name(std::move(name)), params(std::move(params)), body(std::move(body)) {
    }

    bs_obj_ptr accept(interpreter &visitor, context_ptr ctx) override;

    [[nodiscard]] node_ptr copy() const override;
};

struct call_node final : node {
    std::string name;
    std::vector<node_ptr> args;

    call_node(std::string name, std::vector<node_ptr> args, span where) : node(std::move(where)),
                                                                          name(std::move(name)),
                                                                          args(std::move(args)) {
    }

    bs_obj_ptr accept(interpreter &visitor, context_ptr ctx) override;

    [[nodiscard]] node_ptr copy() const override;
};

struct string_node final : node {
    std::string str;

    explicit string_node(const token &t) : node(span{t.start, t.end}), str(t.literal) {
    }

    bs_obj_ptr accept(interpreter &visitor, context_ptr ctx) override;

    [[nodiscard]] node_ptr copy() const override;
};

struct return_node final : node {
    node_ptr expr;

    return_node(node_ptr expr, span where) : node(std::move(where)), expr(std::move(expr)) {
    }

    bs_obj_ptr accept(interpreter &visitor, context_ptr ctx) override;

    [[nodiscard]] node_ptr copy() const override;
};

struct break_node final : node {
    explicit break_node(span where) : node(std::move(where)) {
    }

    bs_obj_ptr accept(interpreter &visitor, context_ptr ctx) override;

    [[nodiscard]] node_ptr copy() const override;
};

struct continue_node final : node {
    explicit continue_node(span where) : node(std::move(where)) {
    }

    bs_obj_ptr accept(interpreter &visitor, context_ptr ctx) override;

    [[nodiscard]] node_ptr copy() const override;
};

#endif //BSHARP_NODES_H
