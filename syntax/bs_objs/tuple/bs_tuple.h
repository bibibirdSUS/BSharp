//
// Created by bibib on 2026/6/17.
//

#ifndef BSHARP_BS_TUPLE_H
#define BSHARP_BS_TUPLE_H


#include "../bs_obj.h"


class bs_tuple : public bs_obj {
public:
    explicit bs_tuple(std::vector<bs_obj_ptr> elements);

    [[nodiscard]] bs_obj_ptr copy() const override;

    [[nodiscard]] std::string type_name() const override;

    bs_obj_ptr subscript(interpreter &visitor, const bs_obj_ptr &index) const override;

    [[nodiscard]] std::string to_string() const override;

    [[nodiscard]] bool to_boolean() const override;

    [[nodiscard]] bs_obj_ptr eq(interpreter &visitor, const bs_obj_ptr &rhs) const override;

    [[nodiscard]] size_t len() const override;

    [[nodiscard]] size_t hash() const override;

    [[nodiscard]] bs_obj_ptr add(interpreter &visitor, const bs_obj_ptr &rhs) const override;

    [[nodiscard]] bs_obj_ptr mul(interpreter &visitor, const bs_obj_ptr &rhs) const override;

    [[nodiscard]] bs_obj_ptr slice(interpreter &visitor, int64_t start, int64_t end, int64_t step) const override;

    size_t size() const;

    bs_obj_ptr get(size_t idx) const;

    [[nodiscard]] std::unique_ptr<bs_iterator> iter() const override;

private:
    const std::vector<bs_obj_ptr> elements_;
};


#endif //BSHARP_BS_TUPLE_H
