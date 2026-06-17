//
// Created by bibib on 2026/6/17.
//

#ifndef BSHARP_BS_TUPLE_ITERATOR_H
#define BSHARP_BS_TUPLE_ITERATOR_H


#include "bs_tuple.h"
#include "../iterator/bs_iterator.h"


class bs_tuple_iterator : public bs_iterator {
public:
    explicit bs_tuple_iterator(const bs_tuple &tuple);

    bs_obj_ptr next() override;

    [[nodiscard]] bool has_next() const override;

private:
    const bs_tuple &tuple_;
    size_t idx_{0};
    size_t length_{0};
};


#endif //BSHARP_BS_TUPLE_ITERATOR_H
