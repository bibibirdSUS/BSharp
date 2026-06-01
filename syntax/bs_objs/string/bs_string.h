//
// Created by bibibird on 2026/4/5.
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

    [[nodiscard]] bs_obj_ptr add(interpreter &visitor, const bs_obj_ptr &rhs) const override;

    [[nodiscard]] bs_obj_ptr mul(interpreter &visitor, const bs_obj_ptr &rhs) const override;

    [[nodiscard]] bs_obj_ptr eq(interpreter &visitor, const bs_obj_ptr &rhs) const override;

    [[nodiscard]] bs_obj_ptr subscript(interpreter &visitor, const bs_obj_ptr &index) const override;

    [[nodiscard]] bs_obj_ptr slice(interpreter &visitor, int start, int end, int step) const override;

    [[nodiscard]] size_t len() const override;

    [[nodiscard]] size_t hash() const override;

private:
    std::string str_;
};


#endif //BSHARP_BS_STRING_H
