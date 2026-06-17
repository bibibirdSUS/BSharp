#include "nodes.h"
#include "../interpreter/interpreter.h"

template<typename T>
std::vector<std::unique_ptr<T> > clone_vector(const std::vector<std::unique_ptr<T> > &vec) {
    std::vector<std::unique_ptr<T> > result;
    result.reserve(vec.size());
    for (const auto &item: vec)
        result.push_back(item ? std::unique_ptr<T>(static_cast<T *>(item->copy().release())) : nullptr);

    return result;
}

// Specialization for node_ptr
inline std::vector<node_ptr> clone_nodes(const std::vector<node_ptr> &vec) {
    std::vector<node_ptr> result;
    result.reserve(vec.size());
    for (const auto &item: vec)
        result.push_back(item ? item->copy() : nullptr);
    return result;
}

bs_obj_ptr number_node::accept(interpreter &visitor, context_ptr ctx) {
    return visitor.visit(*this);
}

node_ptr number_node::copy() const {
    auto n = std::make_unique<number_node>(token{std::to_string(value), token_type::NUMBER, where.start, where.end});
    n->value = value;
    return n;
}

bs_obj_ptr boolean_node::accept(interpreter &visitor, context_ptr ctx) {
    return visitor.visit(*this);
}

node_ptr boolean_node::copy() const {
    return std::make_unique<boolean_node>(token{
        value ? "true" : "false", value ? token_type::TRUE : token_type::FALSE, where.start, where.end
    });
}

bs_obj_ptr null_node::accept(interpreter &visitor, context_ptr ctx) {
    return visitor.visit(*this);
}

node_ptr null_node::copy() const {
    return std::make_unique<null_node>(token{"null", token_type::NULL_LIT, where.start, where.end});
}

bs_obj_ptr comparison_node::accept(interpreter &visitor, const context_ptr ctx) {
    return visitor.visit(*this, ctx);
}

node_ptr comparison_node::copy() const {
    return std::make_unique<comparison_node>(clone_nodes(operands), std::vector<token_type>(operators));
}

bs_obj_ptr bin_op_node::accept(interpreter &visitor, const context_ptr ctx) {
    return visitor.visit(*this, ctx);
}

node_ptr bin_op_node::copy() const {
    return std::make_unique<bin_op_node>(left->copy(), token{"", op, where.start, where.end}, right->copy());
}

bs_obj_ptr unary_op_node::accept(interpreter &visitor, const context_ptr ctx) {
    return visitor.visit(*this, ctx);
}

node_ptr unary_op_node::copy() const {
    return std::make_unique<unary_op_node>(token{"", op, where.start, where.end}, operand->copy());
}

bs_obj_ptr factorial_node::accept(interpreter &visitor, const context_ptr ctx) {
    return visitor.visit(*this, ctx);
}

node_ptr factorial_node::copy() const {
    return std::make_unique<factorial_node>(operand->copy(), token{"!", token_type::FACTORIAL, where.start, where.end});
}

bs_obj_ptr var_assign_node::accept(interpreter &visitor, const context_ptr ctx) {
    return visitor.visit(*this, ctx);
}

node_ptr var_assign_node::copy() const {
    return std::make_unique<var_assign_node>(target->copy(), op, value->copy());
}

bs_obj_ptr var_access_node::accept(interpreter &visitor, const context_ptr ctx) {
    return interpreter::visit(*this, ctx);
}

node_ptr var_access_node::copy() const {
    return std::make_unique<var_access_node>(token{name, token_type::IDENTIFIER, where.start, where.end});
}

bs_obj_ptr statements_node::accept(interpreter &visitor, const context_ptr ctx) {
    return visitor.visit(*this, ctx);
}

node_ptr statements_node::copy() const {
    return std::make_unique<statements_node>(clone_nodes(statements));
}

bs_obj_ptr if_node::accept(interpreter &visitor, const context_ptr ctx) {
    return visitor.visit(*this, ctx);
}

node_ptr if_node::copy() const {
    return std::make_unique<if_node>(condition->copy(), then_branch->copy(),
                                     else_branch ? else_branch->copy() : nullptr, span{where.start, where.end},
                                     is_stmt);
}

bs_obj_ptr while_node::accept(interpreter &visitor, const context_ptr ctx) {
    return visitor.visit(*this, ctx);
}

node_ptr while_node::copy() const {
    return std::make_unique<while_node>(condition->copy(), body->copy(), is_until, span{where.start, where.end});
}

bs_obj_ptr for_node::accept(interpreter &visitor, const context_ptr ctx) {
    return visitor.visit(*this, ctx);
}

node_ptr for_node::copy() const {
    return std::make_unique<for_node>(target->copy(), start->copy(), end->copy(), step ? step->copy() : nullptr,
                                      body->copy(),
                                      span{where.start, where.end});
}

bs_obj_ptr fn_node::accept(interpreter &visitor, const context_ptr ctx) {
    return interpreter::visit(*this, ctx);
}

node_ptr fn_node::copy() const {
    return std::make_unique<fn_node>(name, std::vector(params), body->copy(), span{where.start, where.end});
}

bs_obj_ptr call_node::accept(interpreter &visitor, const context_ptr ctx) {
    return visitor.visit(*this, ctx);
}

node_ptr call_node::copy() const {
    return std::make_unique<call_node>(callee->copy(), clone_nodes(args), span{where.start, where.end});
}

bs_obj_ptr string_node::accept(interpreter &visitor, context_ptr ctx) {
    return visitor.visit(*this);
}

node_ptr string_node::copy() const {
    return std::make_unique<string_node>(token{str, token_type::STRING, where.start, where.end});
}

bs_obj_ptr return_node::accept(interpreter &visitor, const context_ptr ctx) {
    return visitor.visit(*this, ctx);
}

node_ptr return_node::copy() const {
    return std::make_unique<return_node>(expr ? expr->copy() : nullptr, span{where.start, where.end});
}

bs_obj_ptr break_node::accept(interpreter &visitor, const context_ptr ctx) {
    return interpreter::visit(*this);
}

node_ptr break_node::copy() const {
    return std::make_unique<break_node>(span{where.start, where.end});
}

bs_obj_ptr continue_node::accept(interpreter &visitor, const context_ptr ctx) {
    return interpreter::visit(*this);
}

node_ptr continue_node::copy() const {
    return std::make_unique<continue_node>(span{where.start, where.end});
}

bs_obj_ptr list_literal_node::accept(interpreter &visitor, const context_ptr ctx) {
    return visitor.visit(*this, ctx);
}

node_ptr list_literal_node::copy() const {
    return std::make_unique<list_literal_node>(clone_nodes(args), span{where.start, where.end});
}

bs_obj_ptr subscript_node::accept(interpreter &visitor, const context_ptr ctx) {
    return visitor.visit(*this, ctx);
}

node_ptr subscript_node::copy() const {
    return std::make_unique<subscript_node>(left->copy(), index->copy(), span{where.start, where.end});
}

bs_obj_ptr slice_node::accept(interpreter &visitor, const context_ptr ctx) {
    return visitor.visit(*this, ctx);
}

node_ptr slice_node::copy() const {
    return std::make_unique<slice_node>(left->copy(), start->copy(), end->copy(), step->copy(),
                                        span{where.start, where.end});
}

bs_obj_ptr destructuring_assign_node::accept(interpreter &visitor, const context_ptr ctx) {
    return visitor.visit(*this, ctx);
}

node_ptr destructuring_assign_node::copy() const {
    return std::make_unique<destructuring_assign_node>(targets->copy(), right->copy());
}

bs_obj_ptr expr_list_node::accept(interpreter &visitor, const context_ptr ctx) {
    return visitor.visit(*this, ctx);
}

node_ptr expr_list_node::copy() const {
    return std::make_unique<expr_list_node>(clone_nodes(expressions), span{where.start, where.end});
}

bs_obj_ptr foreach_node::accept(interpreter &visitor, context_ptr ctx) {
    return visitor.visit(*this, ctx);
}

node_ptr foreach_node::copy() const {
    return std::make_unique<foreach_node>(target->copy(), container->copy(), body->copy(),
                                          span{where.start, where.end});
}

bs_obj_ptr unpack_node::accept(interpreter &visitor, const context_ptr ctx) {
    return interpreter::visit(*this, ctx);
}

node_ptr unpack_node::copy() const {
    return std::make_unique<unpack_node>(target->copy());
}
