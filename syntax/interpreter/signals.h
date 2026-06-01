//
// Created by bibibird on 2026/5/19.
//

#ifndef BSHARP_SIGNALS_H
#define BSHARP_SIGNALS_H
#include <exception>

#include "context.h"

class break_signal : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "break";
    }
};

class continue_signal : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "continue";
    }
};

class return_signal : public std::exception {
public:
    explicit return_signal(bs_obj_ptr value) : value_(std::move(value)) {
    }

    [[nodiscard]] bs_obj_ptr get_value() const {
        return value_;
    }

private:
    bs_obj_ptr value_;
};


#endif //BSHARP_SIGNALS_H
