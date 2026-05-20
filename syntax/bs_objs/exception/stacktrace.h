//
// Created by bibib on 2026/3/29.
//

#ifndef BSHARP_STACK_FRAME_H
#define BSHARP_STACK_FRAME_H
#include <string>
#include <utility>
#include <vector>

#include "../../position/position.h"

struct stack_frame {
    std::string fn_name;
    position start, end;
};

class call_stack_guard {
public:
    explicit call_stack_guard(std::vector<stack_frame> &call_stack, std::string name, position start_pos,
                              position end_pos)
        : stack_(call_stack) {
        stack_.push_back(stack_frame{std::move(name), std::move(start_pos), std::move(end_pos)});
    }

    ~call_stack_guard() {
        if (!stack_.empty()) stack_.pop_back();
    }

private:
    std::vector<stack_frame> &stack_;
};

#endif //BSHARP_STACK_FRAME_H
