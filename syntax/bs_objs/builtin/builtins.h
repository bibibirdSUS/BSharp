//
// Created by bibib on 2026/5/18.
//

#ifndef BSHARP_BUILTINS_H
#define BSHARP_BUILTINS_H
#include <functional>
#include <iostream>
#include "bs_builtin_function.h"
#include "../../interpreter/context.h"

typedef std::function<bs_obj_ptr(interpreter &, const std::vector<bs_obj_ptr> &)> native_fn;
typedef const std::vector<bs_obj_ptr> arg;

static void register_fn(const std::shared_ptr<context> &global_ctx, const std::string &name, const native_fn &func) {
    global_ctx->define(name, std::make_shared<bs_builtin_function>(name, func));
}

static void register_all(const std::shared_ptr<context> &global_ctx) {
    // print
    register_fn(global_ctx, "print", [](interpreter &visitor, arg &args) {
        if (args.size() > 1)
            for (auto &parm: args)
                std::cout << parm->to_string();
        else if (!args.empty())
            std::cout << args[0]->to_string();

        std::cout << std::endl;
        return visitor.get_runtime().null_obj();
    });
    // input
    register_fn(global_ctx, "input", [](interpreter &visitor, arg &args) {
        if (args.size() > 1)
            throw std::runtime_error{
                "Function 'input' expects 0 or 1 arguments, but " + std::to_string(args.size()) + " were given."
            };

        if (!args.empty()) std::cout << args[0]->to_string() << std::endl;
        std::string input;
        std::getline(std::cin, input);

        return visitor.get_runtime().get_string(input);
    });
    // type
    register_fn(global_ctx, "type", [](interpreter &visitor, arg &args) {
        if (args.size() != 1)
            throw std::runtime_error{
                "Function 'input' expects 1 arguments, but " + std::to_string(args.size()) + " were given."
            };
        return visitor.get_runtime().get_string(args[0]->type_name());
    });
    // assert
    register_fn(global_ctx, "assert", [](interpreter &visitor, arg &args) {
        for (auto &bools: args)
            if (!bools->to_boolean())
                throw std::runtime_error{"Assertion failed"};
        return visitor.get_runtime().null_obj();
    });
}


#endif //BSHARP_BUILTINS_H
