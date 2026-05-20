//
// Created by bibib on 2026/3/7.
//

#ifndef BSHARP_BS_BOOLEAN_H
#define BSHARP_BS_BOOLEAN_H

#include "../bs_obj.h"

class bs_boolean : public bs_obj {
public:
    explicit bs_boolean(bool value);

    [[nodiscard]] bool to_boolean() const override;

    [[nodiscard]] bs_obj_ptr copy() const override;

    [[nodiscard]] std::string to_string() const override;

    [[nodiscard]] std::string type_name() const override;

    [[nodiscard]] bs_obj_ptr eq(const bs_obj &rhs) const override;

private:
    const bool value_;
};

const bs_boolean *as_boolean(const bs_obj &obj);

#endif //BSHARP_BS_BOOLEAN_H
