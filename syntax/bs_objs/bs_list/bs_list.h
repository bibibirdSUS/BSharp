//
// Created by bibib on 2026/5/22.
//

#ifndef BSHARP_BS_LIST_H
#define BSHARP_BS_LIST_H
#include "../bs_obj.h"


class bs_list : public bs_obj {
public:
    explicit bs_list(std::vector<bs_obj_ptr> elements);

    [[nodiscard]] bs_obj_ptr copy() const override;

    [[nodiscard]] std::string type_name() const override;

    bs_obj_ptr subscript(const bs_obj_ptr &index) const override;

    void set_subscript(const bs_obj_ptr &index, const bs_obj_ptr &value) override;

    [[nodiscard]] std::string to_string() const override;

    [[nodiscard]] bool to_boolean() const override;

    [[nodiscard]] bs_obj_ptr eq(const bs_obj_ptr &rhs) const override;

    void append(bs_obj_ptr obj);

    size_t size() const;

    bs_obj_ptr get(size_t idx);

private:
    std::vector<bs_obj_ptr> elements_;
};


#endif //BSHARP_BS_LIST_H
