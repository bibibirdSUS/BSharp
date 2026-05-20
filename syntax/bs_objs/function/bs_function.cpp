//
// Created by bibib on 2026/3/28.
//

#include "bs_function.h"
#include "../../interpreter/interpreter.h"
#include "../../interpreter/context.h"
#include "../../interpreter/signals.h"

bs_obj_ptr bs_function::copy() const {
    return std::make_shared<bs_function>(name_, params_, body_->copy(), closure_);
}

std::string bs_function::type_name() const {
    return "Function";
}

bs_obj_ptr bs_function::call(interpreter &visitor, const std::vector<bs_obj_ptr> &args) const {
    if (args.size() != params_.size())
        throw std::runtime_error{
            "Function '" + name_ + "' expects " + std::to_string(params_.size()) +
            " arguments, but " + std::to_string(args.size()) + " were given."
        };

    // Create a new context for the function execution, with the closure as parent
    const auto function_context = std::make_shared<context>(closure_);

    // Bind parameters to arguments
    for (size_t i = 0; i < params_.size(); ++i)
        function_context->define(params_[i], args[i]);

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
