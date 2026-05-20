//
// Created by bibib on 2026/3/1.
//

#ifndef BSHARP_BS_INVALID_SYNTAX_EXCEPTION_H
#define BSHARP_BS_INVALID_SYNTAX_EXCEPTION_H
#include <utility>

#include "bs_exception.h"

class bs_invalid_syntax_exception : public bs_exception{
public:
    bs_invalid_syntax_exception(std::string msg, std::string source_code, const position &start,
        const position &end)
        : bs_exception(std::move(msg), std::move(source_code), start, end) {
    }

    [[nodiscard]] std::string type_name() const override {
        return "InvalidSyntaxException";
    }
};

#endif //BSHARP_BS_INVALID_SYNTAX_EXCEPTION_H