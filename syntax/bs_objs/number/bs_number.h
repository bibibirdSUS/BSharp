//
// Created by bibibird on 2026/3/1.
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

    [[nodiscard]] size_t hash() const override;

    [[nodiscard]] bs_obj_ptr add(interpreter &visitor, const bs_obj_ptr &rhs) const override;

    [[nodiscard]] bs_obj_ptr sub(interpreter &visitor, const bs_obj_ptr &rhs) const override;

    [[nodiscard]] bs_obj_ptr mul(interpreter &visitor, const bs_obj_ptr &rhs) const override;

    [[nodiscard]] bs_obj_ptr div(interpreter &visitor, const bs_obj_ptr &rhs) const override;

    [[nodiscard]] bs_obj_ptr pow(interpreter &visitor, const bs_obj_ptr &rhs) const override;

    [[nodiscard]] bs_obj_ptr mod(interpreter &visitor, const bs_obj_ptr &rhs) const override;

    [[nodiscard]] bs_obj_ptr fact(interpreter &visitor) const override;

    [[nodiscard]] bs_obj_ptr neg(interpreter &visitor) const override;

    [[nodiscard]] bs_obj_ptr pos(interpreter &visitor) const override;

    [[nodiscard]] bs_obj_ptr eq(interpreter &visitor, const bs_obj_ptr &rhs) const override;

    [[nodiscard]] bs_obj_ptr neq(interpreter &visitor, const bs_obj_ptr &rhs) const override;

    [[nodiscard]] bs_obj_ptr lt(interpreter &visitor, const bs_obj_ptr &rhs) const override;

    [[nodiscard]] bs_obj_ptr gt(interpreter &visitor, const bs_obj_ptr &rhs) const override;

    [[nodiscard]] bs_obj_ptr le(interpreter &visitor, const bs_obj_ptr &rhs) const override;

    [[nodiscard]] bs_obj_ptr ge(interpreter &visitor, const bs_obj_ptr &rhs) const override;

    static bool is_int(const double d) {
        return std::abs(std::floor(d) - d) < eps;
    }

    bool is_int() const {
        return is_int(value_);
    }

    int64_t as_int() const {
        if (!std::isfinite(value_))
            throw std::runtime_error{
                "integer value cannot be NaN or Infinity"
            };

        if (!is_int(value_))
            throw std::runtime_error{
                "expected integer, got " + to_string()
            };

        if (value_ < static_cast<double>(std::numeric_limits<int64_t>::min()) ||
            value_ > static_cast<double>(std::numeric_limits<int64_t>::max()))
            throw std::runtime_error{
                "integer value is outside the int64 range"
            };

        return static_cast<int64_t>(value_);
    }

    [[nodiscard]] double value() const {
        return value_;
    }

private:
    const double value_;
};


#endif //BSHARP_BS_NUMBER_H
