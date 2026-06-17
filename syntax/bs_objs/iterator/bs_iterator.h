//
// Created by bibib on 2026/6/16.
//

#ifndef BSHARP_BS_ITERATOR_H
#define BSHARP_BS_ITERATOR_H

class bs_obj;
typedef std::shared_ptr<bs_obj> bs_obj_ptr;

class bs_iterator {
public:
    virtual ~bs_iterator() = default;

    virtual bs_obj_ptr next() = 0;

    [[nodiscard]] virtual bool has_next() const = 0;
};

#endif //BSHARP_BS_ITERATOR_H
