//
// Created by bibib on 2026/3/1.
//

#include "interpreter.h"

#include <iostream>

#include "context.h"
#include "signals.h"
#include "../bs_objs/exception/bs_runtime_exception.h"
#include "../bs_objs/number/bs_number.h"
#include "../bs_objs/function/bs_function.h"

interpreter::interpreter(node_ptr root, std::string source_code, context_ptr global_context) : source_code_(
        std::move(source_code)),
    root_(std::move(root)), global_context_(std::move(global_context)) {
}

bs_obj_ptr interpreter::eval() {
    call_stack_.push_back(stack_frame{"<main>", root_->where.start, root_->where.end});
    try {
        return eval(root_.get(), global_context_);
    } catch (const break_signal &) {
        throw bs_runtime_exception{
            "'break' outside loop", source_code_, root_->where.start, root_->where.end,
            std::move(call_stack_)
        };
    } catch (const continue_signal &) {
        throw bs_runtime_exception{
            "'continue' outside loop", source_code_, root_->where.start, root_->where.end,
            std::move(call_stack_)
        };
    } catch (const return_signal &) {
        throw bs_runtime_exception{
            "'return' outside function", source_code_, root_->where.start, root_->where.end,
            std::move(call_stack_)
        };
    }
}

bs_obj_ptr interpreter::eval(const node *n, const context_ptr &ctx) {
    if (n == nullptr)
        throw std::runtime_error{"Internal error: null AST node"};

    try {
        return const_cast<node *>(n)->accept(*this, ctx);
    } catch (const std::runtime_error &e) {
        throw bs_runtime_exception{e.what(), source_code_, n->where.start, n->where.end, std::move(call_stack_)};
    }
}

bs_obj_ptr interpreter::visit(const number_node &n) const {
    return rt_.get_number(n.value);
}

bs_obj_ptr interpreter::visit(const boolean_node &n) const {
    return n.value ? rt_.true_obj() : rt_.false_obj();
}

bs_obj_ptr interpreter::visit(const none_node &) const {
    return rt_.null_obj();
}

bs_obj_ptr interpreter::visit(const string_node &n) {
    return rt_.get_string(n.str);
}

bs_obj_ptr interpreter::visit(const comparison_node &n, const context_ptr &ctx) {
    bs_obj_ptr left = eval(n.operands[0].get(), ctx);
    for (size_t i = 0; i < n.operators.size(); ++i) {
        bs_obj_ptr right = eval(n.operands[i + 1].get(), ctx);
        bs_obj_ptr result;

        switch (n.operators[i]) {
            case token_type::EQ: result = left->eq(*right);
                break;
            case token_type::NEQ: result = left->neq(*right);
                break;
            case token_type::LESS: result = left->lt(*right);
                break;
            case token_type::GREATER: result = left->gt(*right);
                break;
            case token_type::LE: result = left->le(*right);
                break;
            case token_type::GE: result = left->ge(*right);
                break;
            default:
                throw std::runtime_error{"Internal error: Unknown comparison operator"};
        }

        if (!result->to_boolean())
            return rt_.false_obj();

        left = std::move(right);
    }
    return rt_.true_obj();
}

bs_obj_ptr interpreter::visit(const bin_op_node &n, const context_ptr &ctx) {
    const bs_obj_ptr left = eval(n.left.get(), ctx);

    // Short-circuiting for AND/OR
    if (n.op == token_type::AND) {
        if (!left->to_boolean())
            return rt_.false_obj();
        return eval(n.right.get(), ctx);
    }
    if (n.op == token_type::OR) {
        if (left->to_boolean())
            return rt_.true_obj();
        return eval(n.right.get(), ctx);
    }

    const bs_obj_ptr right = eval(n.right.get(), ctx);

    switch (n.op) {
        case token_type::PLUS:
            return left->add(*right);
        case token_type::SUBTRACT:
            return left->sub(*right);
        case token_type::MULTIPLY:
            return left->mul(*right);
        case token_type::DIVISION:
            return left->div(*right);
        case token_type::MODULO:
            return left->mod(*right);
        case token_type::POWER:
            return left->pow(*right);
        case token_type::XOR:
            return left->to_boolean() ^ right->to_boolean() ? rt_.true_obj() : rt_.false_obj();

        default:
            throw std::runtime_error{"Internal error: Unknown binary operator"};
    }
}

bs_obj_ptr interpreter::visit(const unary_op_node &n, const context_ptr &ctx) {
    const bs_obj_ptr operand = eval(n.operand.get(), ctx);
    switch (n.op) {
        case token_type::PLUS:
            return operand->pos();
        case token_type::SUBTRACT:
            return operand->neg();
        case token_type::NOT:
            return !operand->to_boolean() ? rt_.true_obj() : rt_.false_obj();
        default:
            throw std::runtime_error{"Internal error: Unknown unary operator"};
    }
}

bs_obj_ptr interpreter::visit(const factorial_node &n, const context_ptr &ctx) {
    return eval(n.operand.get(), ctx)->fact();
}

bs_obj_ptr interpreter::visit(const var_assign_node &n, const context_ptr &ctx) {
    bs_obj_ptr value = eval(n.value.get(), ctx);

    if (n.op == token_type::EQUALS) {
        ctx->set(n.name, value);
        return value;
    }

    // Compound assignment
    const bs_obj_ptr current_val = ctx->get(n.name);
    bs_obj_ptr result;

    switch (n.op) {
        case token_type::PLUS_EQ: result = current_val->add(*value);
            break;
        case token_type::SUB_EQ: result = current_val->sub(*value);
            break;
        case token_type::MUL_EQ: result = current_val->mul(*value);
            break;
        case token_type::DIV_EQ: result = current_val->div(*value);
            break;
        case token_type::MOD_EQ: result = current_val->mod(*value);
            break;
        case token_type::POW_EQ: result = current_val->pow(*value);
            break;
        default:
            throw std::runtime_error{"Internal error: Unknown assignment operator"};
    }

    ctx->set(n.name, result);
    return result;
}

bs_obj_ptr interpreter::visit(const var_access_node &n, const context &ctx) {
    return ctx.get(n.name);
}

bs_obj_ptr interpreter::visit(const statements_node &n, const context_ptr &ctx) {
    bs_obj_ptr result = rt_.null_obj();

    for (const auto &statement: n.statements)
        result = eval(statement.get(), ctx);
    return result;
}

bs_obj_ptr interpreter::visit(const if_node &n, const context_ptr &ctx) {
    const bs_obj_ptr condition = eval(n.condition.get(), ctx);
    bs_obj_ptr result = rt_.null_obj();
    if (condition->to_boolean()) {
        const auto then_context = std::make_shared<context>(ctx);
        result = eval(n.then_branch.get(), then_context);
    } else if (n.else_branch) {
        const auto else_context = std::make_shared<context>(ctx);
        result = eval(n.else_branch.get(), else_context);
    }
    return result;
}

bs_obj_ptr interpreter::visit(const while_node &n, const context_ptr &ctx) {
    bs_obj_ptr result = rt_.null_obj();

    while (true) {
        try {
            if (const bs_obj_ptr condition = eval(n.condition.get(), ctx);
                n.is_until
                    ? condition->to_boolean()
                    : !condition->to_boolean())
                break;

            auto body_context = std::make_shared<context>(ctx);
            result = eval(n.body.get(), body_context);
        } catch (const break_signal &) {
            break;
        } catch (const continue_signal &) {
        }
    }
    return result;
}

bs_obj_ptr interpreter::visit(const for_node &n, const context_ptr &ctx) {
    // Create a new scope for the for loop controller (the variable i)
    auto loop_context = std::make_shared<context>(ctx);

    loop_context->define(n.name, rt_.null_obj());

    bs_obj_ptr result = rt_.null_obj();
    const bs_obj_ptr start = eval(n.start.get(), loop_context);
    const bs_obj_ptr end = eval(n.end.get(), loop_context);

    const bool default_increase = start->lt(*end)->to_boolean();

    while (true) {
        const bs_obj_ptr current_val = loop_context->get(n.name);

        bs_obj_ptr step;
        bool increase;

        if (n.step) {
            step = eval(n.step.get(), loop_context);
            if (step->eq(*rt_.get_number(0))->to_boolean())
                throw std::runtime_error("For loop step cannot be zero");
            increase = step->gt(*rt_.get_number(0))->to_boolean();
        } else {
            step = rt_.get_number(default_increase ? 1.0 : -1.0);
            increase = default_increase;
        }

        if (increase && current_val->ge(*end)->to_boolean()) break;
        if (!increase && current_val->le(*end)->to_boolean()) break;

        try {
            auto iteration_context = std::make_shared<context>(loop_context);
            result = eval(n.body.get(), iteration_context);
        } catch (const break_signal &) {
            break;
        } catch (const continue_signal &) {
            // Fall through to increment
        }

        const bs_obj_ptr val_to_increment = loop_context->get(n.name);
        loop_context->set(n.name, val_to_increment->add(*step));
    }
    return result;
}

bs_obj_ptr interpreter::visit(const fn_node &n, const context_ptr &ctx) {
    bs_obj_ptr fn = std::make_shared<bs_function>(n.name, n.params, n.body->copy(), ctx);
    ctx->define(n.name, fn);
    return fn;
}

bs_obj_ptr interpreter::visit(const call_node &n, const context_ptr &ctx) {
    const bs_obj_ptr func_obj = ctx->get(n.name);

    std::vector<bs_obj_ptr> args;
    for (const auto &arg_node: n.args)
        args.push_back(eval(arg_node.get(), ctx));
    call_stack_guard guard{call_stack_, n.name, n.where.start, n.where.end};
    if (call_stack_.size() > 1000) throw std::runtime_error{"Stack Overflow: Maximum recursion depth exceeded"};
    return func_obj->call(*this, args);
}

bs_obj_ptr interpreter::visit(const return_node &n, const context_ptr &ctx) {
    if (!n.expr)
        throw return_signal{rt_.null_obj()};
    const bs_obj_ptr result = eval(n.expr.get(), ctx);
    throw return_signal{result};
}

bs_obj_ptr interpreter::visit(const break_node &n, const context_ptr &ctx) {
    throw break_signal{};
}

bs_obj_ptr interpreter::visit(const continue_node &n, const context_ptr &ctx) {
    throw continue_signal{};
}
