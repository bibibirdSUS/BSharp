//
// Created by bibibird on 2026/3/7.
//

#ifndef BSHARP_BS_NONE_H
#define BSHARP_BS_NONE_H

#include "../bs_obj.h"

class bs_null : public bs_obj {
public:
    bs_null() = default;

    [[nodiscard]] bs_obj_ptr copy() const override;

    [[nodiscard]] std::string to_string() const override;

    [[nodiscard]] std::string type_name() const override;

    [[nodiscard]] bool to_boolean() const override;

    [[nodiscard]] size_t hash() const override;
};

#endif //BSHARP_BS_NONE_H
