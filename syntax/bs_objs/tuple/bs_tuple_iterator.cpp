//
// Created by bibib on 2026/6/17.
//

#include "bs_tuple_iterator.h"

bs_tuple_iterator::bs_tuple_iterator(const bs_tuple &tuple) : tuple_(tuple), length_(tuple.size()) {
}

bs_obj_ptr bs_tuple_iterator::next() {
    if (!has_next())
        throw std::runtime_error{
            "tuple index out of range: index is " + std::to_string(idx_) +
            ", but size is " + std::to_string(length_)
        };
    return tuple_.get(idx_++);
}

bool bs_tuple_iterator::has_next() const {
    return idx_ < length_;
}
