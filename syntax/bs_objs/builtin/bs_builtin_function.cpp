//
// Created by bibibird on 2026/5/18.
//

#include "bs_builtin_function.h"

bs_builtin_function::bs_builtin_function(std::string name,
                                         std::function<bs_obj_ptr(interpreter &, const std::vector<bs_obj_ptr> &)>
                                         cpp_fn) : name_(std::move(name)), func_(std::move(cpp_fn)) {
}

bs_obj::bs_obj_ptr bs_builtin_function::copy() const {
    return const_cast<bs_builtin_function *>(this)->shared_from_this();
}

bs_obj::bs_obj_ptr bs_builtin_function::call(interpreter &visitor, const std::vector<bs_obj_ptr> &args) const {
    return func_(visitor, args);
}

std::string bs_builtin_function::type_name() const {
    return "Function";
}
