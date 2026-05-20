//
// Created by bibib on 2026/4/5.
//

#include "bs_string.h"

#include "../number/bs_number.h"

bs_string::bs_string(std::string value) : str_(std::move(value)) {
}

bs_obj::bs_obj_ptr bs_string::copy() const {
    return const_cast<bs_string *>(this)->shared_from_this();
}

std::string bs_string::type_name() const {
    return "String";
}

std::string bs_string::to_string() const {
    return str_;
}

bool bs_string::to_boolean() const {
    return !str_.empty();
}

bs_obj::bs_obj_ptr bs_string::add(const bs_obj &rhs) const {
    return std::make_shared<bs_string>(str_ + rhs.to_string());
}

bs_obj::bs_obj_ptr bs_string::mul(const bs_obj &rhs) const {
    if (const auto rhs_casted = dynamic_cast<const bs_number *>(&rhs))
        if (rhs_casted->value() >= 0 && bs_number::is_int(rhs_casted->value())) {
            std::string s;
            s.reserve(str_.size() * static_cast<std::string::size_type>(rhs_casted->value()));
            for (int i = 0; i < rhs_casted->value(); ++i) s += str_;
            return std::make_shared<bs_string>(std::move(s));
        }
    return bs_obj::mul(rhs);
}
