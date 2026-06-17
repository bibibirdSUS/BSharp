//
// Created by bibib on 2026/6/16.
//

#ifndef BSHARP_BS_LIST_ITERATOR_H
#define BSHARP_BS_LIST_ITERATOR_H

#include <memory>

#include "../iterator/bs_iterator.h"

class bs_list;

class bs_list_iterator final : public bs_iterator {
public:
    explicit bs_list_iterator(const bs_list &list);

    bs_obj_ptr next() override;

    [[nodiscard]] bool has_next() const override;

private:
    const bs_list &list_;
    size_t idx_{0};
    size_t length_{0};
};


#endif //BSHARP_BS_LIST_ITERATOR_H
