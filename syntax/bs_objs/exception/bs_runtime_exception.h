//
// Created by bibib on 2026/3/1.
//

#ifndef BSHARP_BS_RUNTIME_EXCEPTION_H
#define BSHARP_BS_RUNTIME_EXCEPTION_H
#include <utility>

#include "bs_exception.h"

class bs_runtime_exception : public bs_exception {
public:
    bs_runtime_exception(const std::string &msg, const std::string &source_code, const position &start,
                      const position &end, std::vector<stack_frame> call_stack)
        : bs_exception(msg, source_code, start, end, std::move(call_stack)) {
    }

    [[nodiscard]] std::string type_name() const override {
        return "RuntimeException";
    }
};

#endif //BSHARP_BS_RUNTIME_EXCEPTION_H
