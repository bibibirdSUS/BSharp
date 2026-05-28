//
// Created by bibib on 2026/3/1.
//

#ifndef BSHARP_BS_ILLEGAL_CHARACTER_EXCEPTION_H
#define BSHARP_BS_ILLEGAL_CHARACTER_EXCEPTION_H
#include <utility>

#include "bs_exception.h"

class bs_illegal_character_exception final : public bs_exception {
public:
    bs_illegal_character_exception(const char c, std::string source_code, position start, position end) : bs_exception(
        "illegal character '" + std::string(1, c) + "'", std::move(source_code), std::move(start), std::move(end)) {
    }

    [[nodiscard]] std::string type_name() const override {
        return "IllegalCharacterException";
    }
};

#endif //BSHARP_BS_ILLEGAL_CHARACTER_EXCEPTION_H
