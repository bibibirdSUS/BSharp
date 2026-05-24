//
// Created by bibib on 2026/4/5.
//
#include "bs_obj.h"
#include "boolean/bs_boolean.h"


bs_obj::bs_obj_ptr bs_obj::eq(const bs_obj_ptr &rhs) const {
    return std::make_shared<bs_boolean>(shared_from_this() == rhs);
}

bs_obj::bs_obj_ptr bs_obj::neq(const bs_obj_ptr &rhs) const {
    return std::make_shared<bs_boolean>(!eq(rhs)->to_boolean());
}
