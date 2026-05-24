//
// Created by bibib on 2026/4/5.
//

#ifndef BSHARP_BS_STRING_H
#define BSHARP_BS_STRING_H
#include "../bs_obj.h"


class bs_string : public bs_obj {
public:
    explicit bs_string(std::string value);

    [[nodiscard]] bs_obj_ptr copy() const override;

    [[nodiscard]] std::string type_name() const override;

    [[nodiscard]] std::string to_string() const override;

    [[nodiscard]] bool to_boolean() const override;

    [[nodiscard]] bs_obj_ptr add(const bs_obj_ptr &rhs) const override;

    [[nodiscard]] bs_obj_ptr mul(const bs_obj_ptr &rhs) const override;

    [[nodiscard]] bs_obj_ptr eq(const bs_obj_ptr &rhs) const override;

    [[nodiscard]] bs_obj_ptr subscript(const bs_obj_ptr &index) const override;

private:
    std::string str_;
};


#endif //BSHARP_BS_STRING_H
