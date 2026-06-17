//
// Created by bibibird on 2026/5/18.
//

#ifndef BSHARP_BUILTINS_H
#define BSHARP_BUILTINS_H

#include <functional>
#include <chrono>
#include <cctype>
#include <cmath>
#include <charconv>
#include <cstdint>
#include <format>
#include <iostream>
#include <limits>
#include <string>

#include "bs_builtin_function.h"
#include "../number/bs_number.h"
#include "../../utils.h"


typedef std::function<bs_obj_ptr(interpreter &, const std::vector<bs_obj_ptr> &)> native_fn;
typedef const std::vector<bs_obj_ptr> arg;

namespace {
    void register_fn(const std::shared_ptr<context> &global_ctx, const std::string &name, const native_fn &func) {
        global_ctx->define(name, std::make_shared<bs_builtin_function>(name, func));
    }

    void expect_arity(const std::string &name, const size_t actual, const size_t expected) {
        if (actual != expected)
            throw std::runtime_error{
                name + "() expects " + std::to_string(expected) + " " + argument_word(expected) +
                ", but got " + std::to_string(actual)
            };
    }

    bool is_supported_numeric_source(const bs_obj_ptr &obj) {
        return obj->type_name() == "String" || obj->type_name() == "Number";
    }

    void ensure_numeric_source(const std::string &name, const bs_obj_ptr &obj, const std::string &target_type) {
        if (!is_supported_numeric_source(obj))
            throw std::runtime_error{
                name + "() cannot convert " + obj->type_name() + " to " + target_type
            };
    }

    void ensure_only_trailing_spaces(const std::string &name, const std::string &value, size_t index,
                                     const std::string &target_type) {
        while (index < value.size()) {
            if (!std::isspace(static_cast<unsigned char>(value[index])))
                throw std::runtime_error{name + "() cannot convert '" + value + "' to " + target_type};
            index++;
        }
    }

    double hash_to_number(const size_t hash) {
        static constexpr size_t max_safe_integer = (1ULL << 53) - 1;
        return static_cast<double>(hash & max_safe_integer);
    }

    void expect_arity_range(const std::string &name, const size_t actual, const size_t min, const size_t max) {
        if (actual < min || actual > max)
            throw std::runtime_error{
                name + "() expects " + std::to_string(min) + " to " + std::to_string(max) +
                " arguments, but got " + std::to_string(actual)
            };
    }

    int64_t expect_integer_arg(const std::string &name, arg &args, const size_t index) {
        const auto number = dynamic_cast<const bs_number *>(args[index].get());
        if (!number)
            throw std::runtime_error{
                name + "() argument " + std::to_string(index + 1) + " must be an integer, got " +
                args[index]->type_name()
            };

        if (!number->is_int())
            throw std::runtime_error{
                name + "() argument " + std::to_string(index + 1) +
                " must be an integer, got " + number->to_string()
            };
        const double value = number->value();
        if (value < static_cast<double>(std::numeric_limits<int64_t>::min()) ||
            value >= static_cast<double>(std::numeric_limits<int64_t>::max()))
            throw std::runtime_error{name + "() argument " + std::to_string(index + 1) + " is outside the int64 range"};

        return static_cast<int64_t>(value);
    }

    int expect_shift_arg(const std::string &name, arg &args, const size_t index) {
        const int64_t shift = expect_integer_arg(name, args, index);
        if (shift < 0 || shift > 63)
            throw std::runtime_error{
                name + "() shift count must be between 0 and 63, got " + std::to_string(shift)
            };
        return static_cast<int>(shift);
    }

    void register_bitwise_binary(const std::shared_ptr<context> &global_ctx, const std::string &name,
                                 const char op) {
        register_fn(global_ctx, name, [name, op](interpreter &visitor, arg &args) {
            expect_arity(name, args.size(), 2);
            const int64_t lhs = expect_integer_arg(name, args, 0);
            const int64_t rhs = expect_integer_arg(name, args, 1);

            int64_t result = 0;
            switch (op) {
                case '&': result = lhs & rhs;
                    break;
                case '|': result = lhs | rhs;
                    break;
                case '^': result = lhs ^ rhs;
                    break;
                default:
                    throw std::runtime_error{"internal error: unknown bitwise builtin"};
            }

            return visitor.get_runtime().get_number(static_cast<double>(result));
        });
    }
}

static void register_all(const std::shared_ptr<context> &global_ctx) {
    register_fn(global_ctx, "print", [](interpreter &visitor, arg &args) {
        for (size_t i = 0; i < args.size(); ++i) {
            std::cout << args[i]->to_string();
            if (i != args.size() - 1) std::cout << " ";
        }

        std::cout << std::endl;
        return visitor.get_runtime().null_obj();
    });

    register_fn(global_ctx, "input", [](interpreter &visitor, arg &args) {
        expect_arity_range("input", args.size(), 0, 1);

        if (!args.empty()) std::cout << args[0]->to_string() << std::endl;
        std::string input;
        std::getline(std::cin, input);

        return visitor.get_runtime().get_string(input);
    });

    register_fn(global_ctx, "type", [](interpreter &visitor, arg &args) {
        expect_arity("type", args.size(), 1);
        return visitor.get_runtime().get_string(args[0]->type_name());
    });

    register_fn(global_ctx, "assert", [](interpreter &visitor, arg &args) {
        for (const auto &condition: args)
            if (!condition->to_boolean())
                throw std::runtime_error{"assertion failed"};
        return visitor.get_runtime().null_obj();
    });

    register_fn(global_ctx, "time", [](interpreter &visitor, arg &args) {
        expect_arity("time", args.size(), 0);

        const auto now = std::chrono::steady_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());

        const auto ms = static_cast<double>(duration.count());
        return visitor.get_runtime().get_number(ms);
    });

    register_fn(global_ctx, "string", [](interpreter &visitor, arg &args) {
        expect_arity("string", args.size(), 1);
        return visitor.get_runtime().get_string(args[0]->to_string());
    });

    register_fn(global_ctx, "int", [](interpreter &visitor, arg &args) {
        expect_arity("int", args.size(), 1);
        ensure_numeric_source("int", args[0], "Integer");

        if (const auto number = dynamic_cast<const bs_number *>(args[0].get())) {
            const int64_t converted = number->as_int();
            return visitor.get_runtime().get_number(static_cast<double>(converted));
        }

        const std::string value = args[0]->to_string();
        const char *start = value.data();
        const char *end = start + value.size();
        int64_t converted = 0;
        const auto [ptr, ec] = std::from_chars(start, end, converted);
        if (ec != std::errc() || ptr == start)
            throw std::runtime_error{"int() cannot convert '" + value + "' to Integer"};
        ensure_only_trailing_spaces("int", value, static_cast<size_t>(ptr - start), "Integer");
        return visitor.get_runtime().get_number(static_cast<double>(converted));
    });

    register_fn(global_ctx, "number", [](interpreter &visitor, arg &args) {
        expect_arity("number", args.size(), 1);
        ensure_numeric_source("number", args[0], "Number");
        size_t index = 0;
        const std::string value = args[0]->to_string();
        try {
            const double converted = std::stod(value, &index);
            ensure_only_trailing_spaces("number", value, index, "Number");
            return visitor.get_runtime().get_number(converted);
        } catch (const std::exception &) {
            throw std::runtime_error{"number() cannot convert '" + value + "' to Number"};
        }
    });

    register_fn(global_ctx, "len", [](interpreter &visitor, arg &args) {
        expect_arity("len", args.size(), 1);
        return visitor.get_runtime().get_number(static_cast<double>(args[0]->len()));
    });

    register_fn(global_ctx, "hash", [](interpreter &visitor, arg &args) {
        expect_arity("hash", args.size(), 1);
        return visitor.get_runtime().get_number(hash_to_number(args[0]->hash()));
    });

    // Bitwise helpers until bitwise operators become syntax.
    register_bitwise_binary(global_ctx, "bit_and", '&');
    register_bitwise_binary(global_ctx, "bit_or", '|');
    register_bitwise_binary(global_ctx, "bit_xor", '^');

    register_fn(global_ctx, "bit_not", [](interpreter &visitor, arg &args) {
        expect_arity("bit_not", args.size(), 1);
        return visitor.get_runtime().get_number(static_cast<double>(~expect_integer_arg("bit_not", args, 0)));
    });
    register_fn(global_ctx, "bnot", [](interpreter &visitor, arg &args) {
        expect_arity("bnot", args.size(), 1);
        return visitor.get_runtime().get_number(static_cast<double>(~expect_integer_arg("bnot", args, 0)));
    });

    register_fn(global_ctx, "lshift", [](interpreter &visitor, arg &args) {
        expect_arity("lshift", args.size(), 2);
        const int64_t value = expect_integer_arg("lshift", args, 0);
        const int shift = expect_shift_arg("lshift", args, 1);
        return visitor.get_runtime().get_number(static_cast<double>(value << shift));
    });
    register_fn(global_ctx, "rshift", [](interpreter &visitor, arg &args) {
        expect_arity("rshift", args.size(), 2);
        const int64_t value = expect_integer_arg("rshift", args, 0);
        const int shift = expect_shift_arg("rshift", args, 1);
        return visitor.get_runtime().get_number(static_cast<double>(value >> shift));
    });

    register_fn(global_ctx, "memory", [](interpreter &visitor, arg &args) {
        expect_arity("memory", args.size(), 1);
        return visitor.get_runtime().get_string(std::format("{:#x}", reinterpret_cast<std::uintptr_t>(args[0].get())));
    });
}

#endif //BSHARP_BUILTINS_H
