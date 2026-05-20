//
// Created by bibib on 2026/2/28.
//

#ifndef BSHARP_BS_EXCEPTION_H
#define BSHARP_BS_EXCEPTION_H
#include "stacktrace.h"
#include "../bs_obj.h"
#include "../../position/position.h"


class bs_exception : public bs_obj, public std::exception {
public:
    bs_exception(std::string msg, std::string source_code, position  start, position  end, std::vector<stack_frame> call_stack = {});

    [[nodiscard]] const char * what() const noexcept override;

    [[nodiscard]] std::string to_string() const override;

    [[nodiscard]] bs_obj_ptr copy() const override;


private:
    const std::string msg_;
    const std::string source_code_;
    const position start_;
    const position end_;
    std::vector<stack_frame> call_stack_;
    mutable std::string what_cache_;
};


#endif //BSHARP_BS_EXCEPTION_H
