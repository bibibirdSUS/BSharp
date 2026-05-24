//
// Created by bibib on 2026/5/18.
//

#ifndef BSHARP_BUILTINS_H
#define BSHARP_BUILTINS_H
#include <functional>
#include <iostream>
#include <chrono>
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
        for (size_t i = 0; i < args.size(); ++i) {
            std::cout << args[i]->to_string();
            if (i != args.size() - 1) std::cout << " ";
        }

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
                "Function 'type' expects 1 arguments, but " + std::to_string(args.size()) + " were given."
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
    // time
    register_fn(global_ctx, "time", [](interpreter &visitor, const std::vector<bs_obj_ptr> &args) {
        if (!args.empty())
            throw std::runtime_error(
                "time() expects 0 arguments, but " + std::to_string(args.size()) + " were given.");

        const auto now = std::chrono::steady_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());

        const auto ms = static_cast<double>(duration.count());
        return visitor.get_runtime().get_number(ms);
    });
    // string (will be removed when string has a ctor)
    register_fn(global_ctx, "string", [](interpreter &visitor, const std::vector<bs_obj_ptr> &args) {
        if (args.size() != 1)
            throw std::runtime_error{
                "string() expects 1 argument, but " + std::to_string(args.size()) + " were given."
            };

        return visitor.get_runtime().get_string(args[0]->to_string());
    });
    // int
    register_fn(global_ctx, "int", [](interpreter &visitor, const std::vector<bs_obj_ptr> &args) {
        if (args.size() != 1)
            throw std::runtime_error{
                "int() expects 1 argument, but " + std::to_string(args.size()) + " were given."
            };
        if (args[0]->type_name() != "String" && args[0]->type_name() != "Number")
            throw std::runtime_error{
                "cannot cast " + args[0]->type_name() + " to an integer"
            };
        return visitor.get_runtime().get_number(std::stoi(args[0]->to_string()));
    });
    // number
    register_fn(global_ctx, "number", [](interpreter &visitor, const std::vector<bs_obj_ptr> &args) {
        if (args.size() != 1)
            throw std::runtime_error{
                "number() expects 1 argument, but " + std::to_string(args.size()) + " were given."
            };
        if (args[0]->type_name() != "String" && args[0]->type_name() != "Number")
            throw std::runtime_error{
                "cannot cast " + args[0]->type_name() + " to a number"
            };
        return visitor.get_runtime().get_number(std::stod(args[0]->to_string()));
    });
}


#endif //BSHARP_BUILTINS_H
