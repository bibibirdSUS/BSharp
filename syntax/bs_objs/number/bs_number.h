//
// Created by bibib on 2026/3/1.
//

#ifndef BSHARP_BS_NUMBER_H
#define BSHARP_BS_NUMBER_H
#include <cmath>

#include "../bs_obj.h"


class bs_number : public bs_obj {
public:
    static constexpr double eps = 1e-9;

    explicit bs_number(double value);

    [[nodiscard]] bs_obj_ptr copy() const override;

    [[nodiscard]] std::string to_string() const override;

    [[nodiscard]] bool to_boolean() const override;

    [[nodiscard]] std::string type_name() const override;

    [[nodiscard]] bs_obj_ptr add(const bs_obj &rhs) const override;

    [[nodiscard]] bs_obj_ptr sub(const bs_obj &rhs) const override;

    [[nodiscard]] bs_obj_ptr mul(const bs_obj &rhs) const override;

    [[nodiscard]] bs_obj_ptr div(const bs_obj &rhs) const override;

    [[nodiscard]] bs_obj_ptr pow(const bs_obj &rhs) const override;

    [[nodiscard]] bs_obj_ptr mod(const bs_obj &rhs) const override;

    [[nodiscard]] bs_obj_ptr fact() const override;

    [[nodiscard]] bs_obj_ptr neg() const override;

    [[nodiscard]] bs_obj_ptr pos() const override;

    [[nodiscard]] bs_obj_ptr eq(const bs_obj &rhs) const override;

    [[nodiscard]] bs_obj_ptr neq(const bs_obj &rhs) const override;

    [[nodiscard]] bs_obj_ptr lt(const bs_obj &rhs) const override;

    [[nodiscard]] bs_obj_ptr gt(const bs_obj &rhs) const override;

    [[nodiscard]] bs_obj_ptr le(const bs_obj &rhs) const override;

    [[nodiscard]] bs_obj_ptr ge(const bs_obj &rhs) const override;

    static bool is_int(const double d) {
        return std::abs(std::floor(d) - d) < eps;
    }

    [[nodiscard]] double value() const {
        return value_;
    }

private:
    const double value_;
};


#endif //BSHARP_BS_NUMBER_H
