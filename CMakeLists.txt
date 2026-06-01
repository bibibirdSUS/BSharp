cmake_minimum_required(VERSION 3.20)
project(BSharp)

set(CMAKE_CXX_STANDARD 20)

add_executable(BSharp main.cpp
        syntax/lexer/lexer.cpp
        syntax/lexer/lexer.h
        syntax/token/token.h
        syntax/token/token_types.h
        syntax/position/position.cpp
        syntax/position/position.h
        syntax/bs_objs/bs_obj.h
        syntax/bs_objs/exception/bs_exception.cpp
        syntax/bs_objs/exception/bs_exception.h
        syntax/bs_objs/exception/bs_illegal_character_exception.h
        syntax/parser/parser.cpp
        syntax/parser/parser.h
        syntax/parser/nodes.h
        syntax/bs_objs/exception/bs_invalid_syntax_exception.h
        syntax/utils.h
        syntax/interpreter/interpreter.cpp
        syntax/interpreter/interpreter.h
        syntax/interpreter/context.cpp
        syntax/interpreter/context.h
        syntax/bs_objs/number/bs_number.cpp
        syntax/bs_objs/number/bs_number.h
        syntax/bs_objs/exception/bs_runtime_exception.h
        syntax/bs_objs/boolean/bs_boolean.cpp
        syntax/bs_objs/null/bs_null.cpp
        syntax/parser/nodes.cpp
        syntax/bs_objs/function/bs_function.cpp
        syntax/bs_objs/function/bs_function.h
        syntax/bs_objs/exception/stacktrace.h
        syntax/bs_objs/string/bs_string.cpp
        syntax/bs_objs/string/bs_string.h
        syntax/bs_objs/bs_obj.cpp
        syntax/interpreter/bs_runtime.cpp
        syntax/interpreter/bs_runtime.h
        syntax/bs_objs/builtin/bs_builtin_function.cpp
        syntax/bs_objs/builtin/bs_builtin_function.h
        syntax/bs_objs/builtin/builtins.h
        syntax/interpreter/signals.h
        syntax/bs_objs/bs_list/bs_list.cpp
        syntax/bs_objs/bs_list/bs_list.h
        syntax/bs_objs/exception/simple_wcwidth.h
)
