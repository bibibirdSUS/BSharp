//
// Created by bibibird on 2026/4/5.
//
#include "bs_obj.h"
#include "../interpreter/interpreter.h"

bs_obj::bs_obj_ptr bs_obj::eq(interpreter &visitor, const bs_obj_ptr &rhs) const {
    return shared_from_this() == rhs ? visitor.get_runtime().true_obj() : visitor.get_runtime().false_obj();
}

bs_obj::bs_obj_ptr bs_obj::neq(interpreter &visitor, const bs_obj_ptr &rhs) const {
    return !eq(visitor, rhs)->to_boolean() ? visitor.get_runtime().true_obj() : visitor.get_runtime().false_obj();
}
