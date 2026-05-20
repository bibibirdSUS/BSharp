//
// Created by bibib on 2026/3/7.
//

#include "bs_null.h"

bs_obj::bs_obj_ptr bs_null::copy() const {
    return const_cast<bs_null *>(this)->shared_from_this();
}

std::string bs_null::to_string() const {
    return "null";
}

std::string bs_null::type_name() const {
    return "Null";
}

bool bs_null::to_boolean() const {
    return false;
}
