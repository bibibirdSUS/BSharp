//
// Created by bibibird on 2026/3/7.
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

    [[nodiscard]] bs_obj_ptr eq(interpreter &visitor, const bs_obj_ptr &rhs) const override;

    [[nodiscard]] size_t hash() const override;

private:
    const bool value_;
};

typedef std::shared_ptr<bs_obj> bs_obj_ptr;


const bs_boolean *as_boolean(const bs_obj_ptr &obj);

#endif //BSHARP_BS_BOOLEAN_H
