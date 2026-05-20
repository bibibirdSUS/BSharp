//
// Created by bibib on 2026/3/7.
//

#include "bs_boolean.h"


const bs_boolean *as_boolean(const bs_obj &obj) {
    return dynamic_cast<const bs_boolean *>(&obj);
}

bs_boolean::bs_boolean(const bool value) : value_(value) {
}

bs_obj::bs_obj_ptr bs_boolean::copy() const {
    return const_cast<bs_boolean *>(this)->shared_from_this();
}

std::string bs_boolean::to_string() const {
    return value_ ? "true" : "false";
}

std::string bs_boolean::type_name() const {
    return "Boolean";
}

bool bs_boolean::to_boolean() const {
    return value_;
}

bs_obj::bs_obj_ptr bs_boolean::eq(const bs_obj &rhs) const {
    const bs_boolean *rhs_casted = as_boolean(rhs);
    if (!rhs_casted) return std::make_shared<bs_boolean>(false);
    return std::make_shared<bs_boolean>(value_ == rhs_casted->value_);
}
