//
// Created by bibibird on 2026/3/6.
//

#include "context.h"
#include <stdexcept>

context::context(context_ptr parent) : parent_(std::move(parent)) {
}

void context::set(const std::string &name, bs_obj::bs_obj_ptr value) {
    if (symbols_.contains(name)) {
        symbols_[name] = std::move(value);
        return;
    }
    if (parent_ && parent_->exists(name)) {
        parent_->set(name, std::move(value));
        return;
    }
    throw std::runtime_error{"undefined variable '" + name + "'"};
}

void context::define(const std::string &name, bs_obj::bs_obj_ptr value) {
    symbols_[name] = std::move(value);
}

bs_obj_ptr context::get(const std::string &name) const {
    if (const auto it = symbols_.find(name); it != symbols_.end())
        return it->second;
    if (parent_)
        return parent_->get(name);
    throw std::runtime_error("undefined variable '" + name + "'");
}

bool context::exists(const std::string &name) const {
    if (symbols_.contains(name)) return true;
    if (parent_) return parent_->exists(name);
    return false;
}
