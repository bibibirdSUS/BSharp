//
// Created by bibib on 2026/3/28.
//

#include "bs_function.h"
#include "../../interpreter/interpreter.h"
#include "../../interpreter/context.h"
#include "../../interpreter/signals.h"

namespace {
    std::string argument_word(const size_t count) {
        return count == 1 ? "argument" : "arguments";
    }
}

bs_obj_ptr bs_function::copy() const {
    return std::make_shared<bs_function>(name_, params_, body_->copy(), closure_);
}

std::string bs_function::type_name() const {
    return "Function";
}

bs_obj_ptr bs_function::call(interpreter &visitor, const std::vector<bs_obj_ptr> &args) const {
    const bool variadic = is_variadic();
    const size_t fixed_size = variadic ? params_.size() - 1 : params_.size();

    if (variadic && args.size() < fixed_size)
        throw std::runtime_error{
            name_ + "() expects at least " + std::to_string(fixed_size) +
            " " + argument_word(fixed_size) + ", but got " + std::to_string(args.size())
        };
    if (!variadic && args.size() != fixed_size)
        throw std::runtime_error{
            name_ + "() expects " + std::to_string(params_.size()) +
            " " + argument_word(fixed_size) + ", but got " + std::to_string(args.size())
        };

    // Create a new context for the function execution, with the closure as parent
    const auto function_context = std::make_shared<context>(closure_);

    // Bind parameters to arguments
    for (size_t i = 0; i < fixed_size; ++i)
        function_context->define(params_[i].name, args[i]);

    if (variadic) {
        std::vector<bs_obj_ptr> rest_args;
        for (size_t i = fixed_size; i < args.size(); ++i)
            rest_args.push_back(args[i]);
        function_context->define(params_.back().name, bs_runtime::get_list(rest_args));
    }

    // Execute the function body
    try {
        visitor.eval(body_.get(), function_context);
        return visitor.get_runtime().null_obj();
    } catch (const return_signal &s) {
        return s.get_value();
    } catch (const break_signal &) {
        throw std::runtime_error{"'break' outside loop"};
    } catch (const continue_signal &) {
        throw std::runtime_error{"'continue' outside loop"};
    }
}
