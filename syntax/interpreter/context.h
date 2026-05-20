//
// Created by bibib on 2026/3/6.
//

#ifndef BSHARP_CONTEXT_H
#define BSHARP_CONTEXT_H

#include <string>
#include <unordered_map>
#include <memory>

#include "../bs_objs/bs_obj.h"

typedef bs_obj::bs_obj_ptr bs_obj_ptr;

class context {
public:
    typedef std::shared_ptr<context> context_ptr;

    explicit context(context_ptr parent = nullptr);

    void set(const std::string &name, bs_obj::bs_obj_ptr value);

    void define(const std::string &name, bs_obj::bs_obj_ptr value);

    [[nodiscard]] bs_obj::bs_obj_ptr get(const std::string &name) const;

    [[nodiscard]] bool exists(const std::string &name) const;

private:
    context_ptr parent_;
    std::unordered_map<std::string, bs_obj::bs_obj_ptr> symbols_;
};

#endif //BSHARP_CONTEXT_H
