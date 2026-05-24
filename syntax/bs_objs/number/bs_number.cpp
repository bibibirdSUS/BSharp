//
// Created by bibib on 2026/3/1.
//

#include "bs_number.h"

#include <cmath>

#include "../../utils.h"
#include "../boolean/bs_boolean.h"

const bs_number *as_number(const bs_obj_ptr &obj) {
    return dynamic_cast<const bs_number *>(obj.get());
}

bs_number::bs_number(const double value) : value_(value) {
}

bs_obj::bs_obj_ptr bs_number::copy() const {
    return const_cast<bs_number *>(this)->shared_from_this();
}

std::string bs_number::to_string() const {
    std::ostringstream oss;
    oss << value_;
    return oss.str();
}

std::string bs_number::type_name() const {
    return "Number";
}

bs_obj::bs_obj_ptr bs_number::add(const bs_obj_ptr &rhs) const {
    const bs_number *rhs_casted = as_number(rhs);
    if (!rhs_casted) return bs_obj::add(rhs);
    return std::make_shared<bs_number>(value_ + rhs_casted->value_);
}

bs_obj::bs_obj_ptr bs_number::sub(const bs_obj_ptr &rhs) const {
    const bs_number *rhs_casted = as_number(rhs);
    if (!rhs_casted) return bs_obj::sub(rhs);
    return std::make_shared<bs_number>(value_ - rhs_casted->value_);
}

bs_obj::bs_obj_ptr bs_number::mul(const bs_obj_ptr &rhs) const {
    const bs_number *rhs_casted = as_number(rhs);
    if (!rhs_casted) return bs_obj::mul(rhs);
    return std::make_shared<bs_number>(value_ * rhs_casted->value_);
}

bs_obj::bs_obj_ptr bs_number::div(const bs_obj_ptr &rhs) const {
    const bs_number *rhs_casted = as_number(rhs);
    if (!rhs_casted) return bs_obj::div(rhs);
    if (rhs_casted->value_ == 0) throw std::runtime_error{"Division by zero"};
    return std::make_shared<bs_number>(value_ / rhs_casted->value_);
}

bs_obj::bs_obj_ptr bs_number::pow(const bs_obj_ptr &rhs) const {
    const bs_number *rhs_casted = as_number(rhs);
    if (!rhs_casted) return bs_obj::pow(rhs);
    const double r = std::pow(value_, rhs_casted->value_);
    if (std::isnan(r) || std::isinf(r))
        throw std::runtime_error{
            "Power of number too large or no definition in real numbers"
        };
    return std::make_shared<bs_number>(r);
}

bs_obj::bs_obj_ptr bs_number::mod(const bs_obj_ptr &rhs) const {
    const bs_number *rhs_casted = as_number(rhs);
    if (!rhs_casted) return bs_obj::mod(rhs);
    if (rhs_casted->value_ == 0) throw std::runtime_error{"Division by zero"};
    if (!(std::abs(value_ - std::round(value_)) < eps) || !(
            std::abs(rhs_casted->value_ - std::round(rhs_casted->value_)) < eps))
        throw std::runtime_error{"Modulo of non-integer number is not defined"};
    return std::make_shared<bs_number>(static_cast<int>(value_) % static_cast<int>(rhs_casted->value_));
}

bs_obj::bs_obj_ptr bs_number::fact() const {
    if (!std::isfinite(value_)) throw std::runtime_error{"Factorial of infinite number is not defined"};
    if (value_ < 0 && std::abs(value_ - std::round(value_)) < eps)
        throw std::runtime_error{
            "Factorial of negative integer is not defined"
        };
    const double r = factorial(value_);
    if (!std::isfinite(r)) throw std::runtime_error{"Factorial of number too large"};
    return std::make_shared<bs_number>(r);
}

bs_obj::bs_obj_ptr bs_number::neg() const {
    return std::make_shared<bs_number>(-value_);
}

bs_obj::bs_obj_ptr bs_number::pos() const {
    return std::make_shared<bs_number>(value_);
}

bs_obj::bs_obj_ptr bs_number::eq(const bs_obj_ptr &rhs) const {
    const bs_number *rhs_casted = as_number(rhs);
    if (!rhs_casted) return std::make_shared<bs_boolean>(false);
    return std::make_shared<bs_boolean>(std::abs(value_ - rhs_casted->value_) < eps);
}

bs_obj::bs_obj_ptr bs_number::neq(const bs_obj_ptr &rhs) const {
    const bs_number *rhs_casted = as_number(rhs);
    if (!rhs_casted) return std::make_shared<bs_boolean>(true);
    return std::make_shared<bs_boolean>(std::abs(value_ - rhs_casted->value_) >= eps);
}

bs_obj::bs_obj_ptr bs_number::lt(const bs_obj_ptr &rhs) const {
    const bs_number *rhs_casted = as_number(rhs);
    if (!rhs_casted) return bs_obj::lt(rhs);
    return std::make_shared<bs_boolean>(value_ < rhs_casted->value_);
}

bs_obj::bs_obj_ptr bs_number::gt(const bs_obj_ptr &rhs) const {
    const bs_number *rhs_casted = as_number(rhs);
    if (!rhs_casted) return bs_obj::gt(rhs);
    return std::make_shared<bs_boolean>(value_ > rhs_casted->value_);
}

bs_obj::bs_obj_ptr bs_number::le(const bs_obj_ptr &rhs) const {
    const bs_number *rhs_casted = as_number(rhs);
    if (!rhs_casted) return bs_obj::le(rhs);
    return std::make_shared<bs_boolean>(value_ <= rhs_casted->value_);
}

bs_obj::bs_obj_ptr bs_number::ge(const bs_obj_ptr &rhs) const {
    const bs_number *rhs_casted = as_number(rhs);
    if (!rhs_casted) return bs_obj::ge(rhs);
    return std::make_shared<bs_boolean>(value_ >= rhs_casted->value_);
}

bool bs_number::to_boolean() const {
    return std::abs(value_) > eps;
}
