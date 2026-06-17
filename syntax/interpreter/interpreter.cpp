//
// Created by bibibird on 2026/3/1.
//

#include "interpreter.h"

#include <iostream>
#include <limits>
#include <utility>

#include "context.h"
#include "signals.h"
#include "../utils.h"
#include "../bs_objs/exception/bs_runtime_exception.h"
#include "../bs_objs/number/bs_number.h"
#include "../bs_objs/function/bs_function.h"

namespace {
    const bs_number *expect_slice_number(const bs_obj_ptr &value, const std::string &name) {
        if (const auto number = dynamic_cast<const bs_number *>(value.get()))
            return number;
        throw std::runtime_error{"slice " + name + " must be a number, got " + value->type_name()};
    }

    int64_t expect_slice_integer(const bs_obj_ptr &value, const std::string &name) {
        const auto number = expect_slice_number(value, name);
        if (!number->is_int())
            throw std::runtime_error{"slice " + name + " must be an integer, got " + number->to_string()};
        return number->as_int();
    }

    std::vector<bs_obj_ptr> collect_iterable(const bs_obj_ptr &ptr) {
        const auto it = ptr->iter();
        std::vector<bs_obj_ptr> result;

        while (it->has_next()) result.push_back(it->next());

        return result;
    }
}

void interpreter::assign_target(const node *target, const bs_obj_ptr &value, const context_ptr &ctx, const assign_actions action) {
    if (!is_assignable(target))
        throw std::runtime_error{"invalid assignment target"};

    if (const auto var = dynamic_cast<const var_access_node *>(target))
        switch (action) {
            case assign_actions::DEFINE:
                ctx->define(var->name, value);
                break;
            case assign_actions::SET:
                ctx->set(var->name, value);
                break;
        }

    else if (const auto sub = dynamic_cast<const subscript_node *>(target)) {
        const bs_obj_ptr left = eval(sub->left.get(), ctx);
        const bs_obj_ptr idx = eval(sub->index.get(), ctx);
        left->set_subscript(*this, idx, value);
    } else if (const auto expr_list = dynamic_cast<const expr_list_node *>(target)) {
        const std::vector<bs_obj_ptr> values = collect_iterable(value);

        const size_t target_count = expr_list->expressions.size();

        if (const size_t value_count = values.size(); target_count != value_count)
            throw std::runtime_error{
                "cannot destructure " +
                std::to_string(value_count) +
                "values into " +
                std::to_string(target_count) +
                " targets"
            };
        for (size_t i = 0; i < target_count; ++i)
            assign_target(
                expr_list->expressions[i].get(),
                values[i],
                ctx, action
            );
    }
}

interpreter::interpreter(node_ptr root, std::string source_code, context_ptr global_context) : source_code_(
        std::move(source_code)),
    root_(std::move(root)), global_context_(std::move(global_context)),
    current_span_{position{"", 0, 0}, position{"", 0, 0}} {
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
        throw std::runtime_error{"internal error: null AST node"};

    const span prev_span = current_span_;
    current_span_ = n->where;

    try {
        return const_cast<node *>(n)->accept(*this, ctx);
    } catch (const std::runtime_error &e) {
        throw bs_runtime_exception{e.what(), source_code_, n->where.start, n->where.end, std::move(call_stack_)};
    } catch (...) {
        current_span_ = prev_span;
        throw;
    }
}

[[noreturn]] void interpreter::throw_runtime_error(const std::string &msg) const {
    throw bs_runtime_exception{msg, source_code_, current_span_.start, current_span_.end, call_stack_};
}

bs_obj_ptr interpreter::visit(const number_node &n) const {
    return rt_.get_number(n.value);
}

bs_obj_ptr interpreter::visit(const boolean_node &n) const {
    return n.value ? rt_.true_obj() : rt_.false_obj();
}

bs_obj_ptr interpreter::visit(const null_node &) const {
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
            case token_type::EQ: result = left->eq(*this, right);
                break;
            case token_type::NEQ: result = left->neq(*this, right);
                break;
            case token_type::LESS: result = left->lt(*this, right);
                break;
            case token_type::GREATER: result = left->gt(*this, right);
                break;
            case token_type::LE: result = left->le(*this, right);
                break;
            case token_type::GE: result = left->ge(*this, right);
                break;
            default:
                throw std::runtime_error{"internal error: unknown comparison operator"};
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
            return left->add(*this, right);
        case token_type::SUBTRACT:
            return left->sub(*this, right);
        case token_type::MULTIPLY:
            return left->mul(*this, right);
        case token_type::DIVISION:
            return left->div(*this, right);
        case token_type::MODULO:
            return left->mod(*this, right);
        case token_type::POWER:
            return left->pow(*this, right);
        case token_type::XOR:
            return left->to_boolean() ^ right->to_boolean() ? rt_.true_obj() : rt_.false_obj();

        default:
            throw std::runtime_error{"internal error: unknown binary operator"};
    }
}

bs_obj_ptr interpreter::visit(const unary_op_node &n, const context_ptr &ctx) {
    const bs_obj_ptr operand = eval(n.operand.get(), ctx);
    switch (n.op) {
        case token_type::PLUS:
            return operand->pos(*this);
        case token_type::SUBTRACT:
            return operand->neg(*this);
        case token_type::NOT:
            return !operand->to_boolean() ? rt_.true_obj() : rt_.false_obj();
        default:
            throw std::runtime_error{"internal error: unknown unary operator"};
    }
}

bs_obj_ptr interpreter::visit(const factorial_node &n, const context_ptr &ctx) {
    return eval(n.operand.get(), ctx)->fact(*this);
}

bs_obj_ptr interpreter::visit(const var_assign_node &n, const context_ptr &ctx) {
    bs_obj_ptr value = eval(n.value.get(), ctx);

    if (n.op == token_type::EQUALS) {
        assign_target(n.target.get(), value, ctx, assign_actions::DEFINE);
        return value;
    }

    // Compound assignment
    const bs_obj_ptr current_val = eval(n.target.get(), ctx);
    bs_obj_ptr result;

    switch (n.op) {
        case token_type::PLUS_EQ: result = current_val->add(*this, value);
            break;
        case token_type::SUB_EQ: result = current_val->sub(*this, value);
            break;
        case token_type::MUL_EQ: result = current_val->mul(*this, value);
            break;
        case token_type::DIV_EQ: result = current_val->div(*this, value);
            break;
        case token_type::MOD_EQ: result = current_val->mod(*this, value);
            break;
        case token_type::POW_EQ: result = current_val->pow(*this, value);
            break;
        default:
            throw std::runtime_error{"internal error: unknown assignment operator"};
    }

    assign_target(n.target.get(), result, ctx, assign_actions::SET);
    return result;
}

bs_obj_ptr interpreter::visit(const var_access_node &n, const context_ptr &ctx) {
    return ctx->get(n.name);
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

    return n.is_stmt ? rt_.null_obj() : result;
}

bs_obj_ptr interpreter::visit(const while_node &n, const context_ptr &ctx) {
    for (;;) {
        try {
            if (const bs_obj_ptr condition = eval(n.condition.get(), ctx);
                n.is_until
                    ? condition->to_boolean()
                    : !condition->to_boolean())
                break;

            auto body_context = std::make_shared<context>(ctx);
            eval(n.body.get(), body_context);
        } catch (const break_signal &) {
            break;
        } catch (const continue_signal &) {
        }
    }
    return rt_.null_obj();
}

bs_obj_ptr interpreter::visit(const for_node &n, const context_ptr &ctx) {
    // Create a new scope for the for loop controller (the variable i)
    auto loop_context = std::make_shared<context>(ctx);

    const auto var = dynamic_cast<var_access_node *>(n.target.get());

    const std::string &name = var->name;

    const bs_obj_ptr start = eval(n.start.get(), loop_context);
    const bs_obj_ptr end = eval(n.end.get(), loop_context);

    const bool default_increase = start->lt(*this, end)->to_boolean();

    loop_context->define(name, start);

    for (;;) {
        const bs_obj_ptr current_val = loop_context->get(name);

        bs_obj_ptr step;
        bool increase;

        if (n.step) {
            step = eval(n.step.get(), loop_context);
            if (step->eq(*this, rt_.get_number(0))->to_boolean()) {
                throw bs_runtime_exception{
                    "for loop step cannot be zero",
                    source_code_,
                    n.step->where.start,
                    n.step->where.end,
                    call_stack_
                };
            }
            increase = step->gt(*this, rt_.get_number(0))->to_boolean();
        } else {
            step = rt_.get_number(default_increase ? 1.0 : -1.0);
            increase = default_increase;
        }

        if (increase && current_val->ge(*this, end)->to_boolean()) break;
        if (!increase && current_val->le(*this, end)->to_boolean()) break;

        try {
            auto iteration_context = std::make_shared<context>(loop_context);
            eval(n.body.get(), iteration_context);
        } catch (const break_signal &) {
            break;
        } catch (const continue_signal &) {
            // Fall through to increment
        }

        const bs_obj_ptr val_to_increment = loop_context->get(name);
        loop_context->set(name, val_to_increment->add(*this, step));
    }
    return rt_.null_obj();
}

bs_obj_ptr interpreter::visit(const fn_node &n, const context_ptr &ctx) {
    bs_obj_ptr fn = bs_runtime::get_function(n.name, n.params, n.body->copy(), ctx);
    ctx->define(n.name, fn);
    return fn;
}

bs_obj_ptr interpreter::visit(const call_node &n, const context_ptr &ctx) {
    const bs_obj_ptr func_obj = eval(n.callee.get(), ctx);

    std::vector<bs_obj_ptr> args;
    for (const auto &arg_node: n.args)
        if (const auto unpack = dynamic_cast<const unpack_node *>(arg_node.get())) {
            const bs_obj_ptr evaluated_result = eval(unpack->target.get(), ctx);
            const auto it = evaluated_result->iter();
            while (it->has_next())
                args.push_back(it->next());
        } else args.push_back(eval(arg_node.get(), ctx));

    std::string call_name = "<anonymous>";
    if (const auto var_node = dynamic_cast<const var_access_node *>(n.callee.get()))
        call_name = var_node->name;
    else if (dynamic_cast<const call_node *>(n.callee.get()))
        call_name = "<call result>";
    else if (dynamic_cast<const subscript_node *>(n.callee.get()))
        call_name = "<subscript result>";


    call_stack_guard guard{call_stack_, std::move(call_name), n.where.start, n.where.end};
    if (call_stack_.size() > 1000) throw std::runtime_error{"stack overflow: maximum recursion depth exceeded"};
    return func_obj->call(*this, args);
}

bs_obj_ptr interpreter::visit(const return_node &n, const context_ptr &ctx) {
    if (!n.expr)
        throw return_signal{rt_.null_obj()};
    const bs_obj_ptr result = eval(n.expr.get(), ctx);
    throw return_signal{result};
}

bs_obj_ptr interpreter::visit(const break_node &_) {
    throw break_signal{};
}

bs_obj_ptr interpreter::visit(const continue_node &_) {
    throw continue_signal{};
}

bs_obj_ptr interpreter::visit(const list_literal_node &n, const context_ptr &ctx) {
    std::vector<bs_obj_ptr> evaluated_args;
    evaluated_args.reserve(n.args.size());

    for (const auto &e: n.args) {
        bs_obj_ptr evaluated = eval(e.get(), ctx);
        evaluated_args.push_back(evaluated);
    }
    return bs_runtime::get_list(std::move(evaluated_args));
}

bs_obj_ptr interpreter::visit(const subscript_node &n, const context_ptr &ctx) {
    const bs_obj_ptr left = eval(n.left.get(), ctx);
    const bs_obj_ptr index = eval(n.index.get(), ctx);
    return left->subscript(*this, index);
}

bs_obj_ptr interpreter::visit(const slice_node &n, const context_ptr &ctx) {
    const bs_obj_ptr target_val = eval(n.left.get(), ctx);
    if (!target_val)
        throw std::runtime_error{"cannot slice null object"};

    const size_t target_length = target_val->len();
    if (target_length > static_cast<size_t>(std::numeric_limits<int64_t>::max()))
        throw std::runtime_error{"slice target is too large"};
    const auto length = static_cast<int64_t>(target_length);

    const bs_obj_ptr start_ptr = n.start ? eval(n.start.get(), ctx) : nullptr;
    const bs_obj_ptr end_ptr = n.end ? eval(n.end.get(), ctx) : nullptr;
    const bs_obj_ptr step_ptr = n.step ? eval(n.step.get(), ctx) : nullptr;

    int64_t actual_step = 1;
    if (step_ptr)
        actual_step = expect_slice_integer(step_ptr, "step");

    if (actual_step == 0) {
        if (n.step) {
            throw bs_runtime_exception{
                "slice step cannot be zero",
                source_code_,
                n.step->where.start,
                n.step->where.end,
                call_stack_
            };
        }
        throw_runtime_error("slice step cannot be zero");
    }

    const bool positive_step = actual_step > 0;

    int64_t actual_start =
            start_ptr
                ? expect_slice_integer(start_ptr, "start")
                : positive_step
                      ? 0
                      : length - 1;

    int64_t actual_end =
            end_ptr
                ? expect_slice_integer(end_ptr, "end")
                : positive_step
                      ? length
                      : -1;

    if (actual_start < 0) actual_start += length;
    if (actual_end < 0 && (positive_step || end_ptr))
        actual_end += length;


    const int64_t lower = positive_step ? 0 : -1;
    const int64_t upper = positive_step ? length : length - 1;

    actual_start = clamp(lower, upper, actual_start);
    actual_end = clamp(lower, upper, actual_end);

    return target_val->slice(*this, actual_start, actual_end, actual_step);
}

bs_obj_ptr interpreter::visit(const destructuring_assign_node &n, const context_ptr &ctx) {
    const bs_obj_ptr value = eval(n.right.get(), ctx);

    assign_target(n.targets.get(), value, ctx, assign_actions::DEFINE);

    return value;
}

bs_obj_ptr interpreter::visit(const expr_list_node &n, const context_ptr &ctx) {
    std::vector<bs_obj_ptr> values;
    values.reserve(n.expressions.size());

    for (const auto &expr: n.expressions)
        values.push_back(eval(expr.get(), ctx));

    return bs_runtime::get_tuple(std::move(values));
}

bs_obj_ptr interpreter::visit(const foreach_node &n, const context_ptr &ctx) {
    const bs_obj_ptr container = eval(n.container.get(), ctx);

    const std::unique_ptr<bs_iterator> it = container->iter();

    while (it->has_next()) {
        try {
            auto local_ctx = std::make_shared<context>(ctx);
            assign_target(n.target.get(), it->next(), local_ctx, assign_actions::DEFINE);
            eval(n.body.get(), local_ctx);
        } catch (const break_signal &) {
            break;
        } catch (const continue_signal &) {
        }
    }

    return rt_.null_obj();
}

bs_obj_ptr interpreter::visit(const unpack_node &n, const context_ptr &ctx) {
    throw std::runtime_error{"internal error: unpack_node shouldn't be visited"};
}
