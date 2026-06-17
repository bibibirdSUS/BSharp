//
// Created by bibib on 2026/6/16.
//

#include "bs_list_iterator.h"

#include "bs_list.h"

bs_list_iterator::bs_list_iterator(const bs_list &list) : list_(list), length_(list.len()) {
}

bs_obj_ptr bs_list_iterator::next() {
    if (!has_next())
        throw std::runtime_error{
            "list index out of range: index is " + std::to_string(idx_) +
            ", but size is " + std::to_string(length_)
        };
    return list_.get(idx_++);
}

bool bs_list_iterator::has_next() const {
    return idx_ < length_;
}
