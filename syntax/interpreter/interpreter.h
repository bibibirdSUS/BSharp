//
// Created by bibibird on 2026/3/1.
//

#ifndef BSHARP_INTERPRETER_H
#define BSHARP_INTERPRETER_H
#include <string>


#include "bs_runtime.h"
#include "../bs_objs/bs_obj.h"
#include "../bs_objs/exception/stacktrace.h"
#include "../parser/nodes.h"


typedef bs_obj::bs_obj_ptr bs_obj_ptr;

class interpreter {
public:
    typedef context::context_ptr context_ptr;

    interpreter(node_ptr root, std::string source_code, context_ptr global_context);

    bs_obj_ptr eval();

    bs_obj_ptr eval(const node *n, const context_ptr &ctx);

    bs_obj_ptr visit(const number_node &n) const;

    bs_obj_ptr visit(const boolean_node &n) const;

    bs_obj_ptr visit(const null_node &n) const;

    bs_obj_ptr visit(const string_node &n);

    bs_obj_ptr visit(const statements_node &n, const context_ptr &ctx);

    bs_obj_ptr visit(const comparison_node &n, const context_ptr &ctx);

    bs_obj_ptr visit(const bin_op_node &n, const context_ptr &ctx);

    bs_obj_ptr visit(const unary_op_node &n, const context_ptr &ctx);

    bs_obj_ptr visit(const factorial_node &n, const context_ptr &ctx);

    bs_obj_ptr visit(const var_assign_node &n, const context_ptr &ctx);

    static bs_obj_ptr visit(const var_access_node &n, const context_ptr &ctx);

    bs_obj_ptr visit(const if_node &n, const context_ptr &ctx);

    bs_obj_ptr visit(const while_node &n, const context_ptr &ctx);

    bs_obj_ptr visit(const for_node &n, const context_ptr &ctx);

    bs_obj_ptr visit(const call_node &n, const context_ptr &ctx);

    bs_obj_ptr visit(const return_node &n, const context_ptr &ctx);

    static bs_obj_ptr visit(const break_node &_);

    static bs_obj_ptr visit(const continue_node &_);

    static bs_obj_ptr visit(const fn_node &n, const context_ptr &ctx);

    bs_obj_ptr visit(const list_literal_node &n, const context_ptr &ctx);

    bs_obj_ptr visit(const subscript_node &n, const context_ptr &ctx);

    bs_obj_ptr visit(const subscript_assign_node &n, const context_ptr &ctx);

    bs_obj_ptr visit(const slice_node &n, const context_ptr &ctx);

    bs_runtime &get_runtime() { return rt_; }

private:
    const std::string source_code_;
    node_ptr root_;
    context_ptr global_context_;
    std::vector<stack_frame> call_stack_;
    bs_runtime rt_;
};


#endif //BSHARP_INTERPRETER_H
