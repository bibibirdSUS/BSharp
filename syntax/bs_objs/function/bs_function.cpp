//
// Created by bibibird on 2026/3/28.
//

#include "bs_function.h"
#include "../../utils.h"
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
    const bool variadic = is_variadic();
    const size_t fixed_param_count = variadic ? params_.size() - 1 : params_.size();

    if (variadic && args.size() < fixed_param_count)
        throw std::runtime_error{
            name_ + "() expects at least " + std::to_string(fixed_param_count) +
            " " + argument_word(fixed_param_count) + ", but got " + std::to_string(args.size())
        };
    if (!variadic && args.size() != fixed_param_count)
        throw std::runtime_error{
            name_ + "() expects " + std::to_string(params_.size()) +
            " " + argument_word(params_.size()) + ", but got " + std::to_string(args.size())
        };

    const auto function_context = std::make_shared<context>(closure_);

    for (size_t i = 0; i < fixed_param_count; ++i)
        function_context->define(params_[i].t.literal, args[i]);

    if (variadic) {
        std::vector<bs_obj_ptr> rest_args;
        for (size_t i = fixed_param_count; i < args.size(); ++i)
            rest_args.push_back(args[i]);
        function_context->define(params_.back().t.literal, bs_runtime::get_list(std::move(rest_args)));
    }

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
