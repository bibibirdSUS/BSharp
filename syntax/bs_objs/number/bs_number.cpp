//
// Created by bibibird on 2026/3/1.
//

#include "bs_number.h"

#include <functional>
#include <cmath>

#include "../../utils.h"
#include "../boolean/bs_boolean.h"
#include "../../interpreter/interpreter.h"

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

size_t bs_number::hash() const {
    return std::hash<std::string>{}("Number");
}

bs_obj::bs_obj_ptr bs_number::add(interpreter &visitor, const bs_obj_ptr &rhs) const {
    const bs_number *rhs_casted = as_number(rhs);
    if (!rhs_casted) return bs_obj::add(visitor, rhs);
    return visitor.get_runtime().get_number(value_ + rhs_casted->value_);
}

bs_obj::bs_obj_ptr bs_number::sub(interpreter &visitor, const bs_obj_ptr &rhs) const {
    const bs_number *rhs_casted = as_number(rhs);
    if (!rhs_casted) return bs_obj::sub(visitor, rhs);
    return visitor.get_runtime().get_number(value_ - rhs_casted->value_);
}

bs_obj::bs_obj_ptr bs_number::mul(interpreter &visitor, const bs_obj_ptr &rhs) const {
    const bs_number *rhs_casted = as_number(rhs);
    if (!rhs_casted) return bs_obj::mul(visitor, rhs);
    return visitor.get_runtime().get_number(value_ * rhs_casted->value_);
}

bs_obj::bs_obj_ptr bs_number::div(interpreter &visitor, const bs_obj_ptr &rhs) const {
    const bs_number *rhs_casted = as_number(rhs);
    if (!rhs_casted) return bs_obj::div(visitor, rhs);
    if (rhs_casted->value_ == 0) throw std::runtime_error{"division by zero"};
    return visitor.get_runtime().get_number(value_ / rhs_casted->value_);
}

bs_obj::bs_obj_ptr bs_number::pow(interpreter &visitor, const bs_obj_ptr &rhs) const {
    const bs_number *rhs_casted = as_number(rhs);
    if (!rhs_casted) return bs_obj::pow(visitor, rhs);
    const double r = std::pow(value_, rhs_casted->value_);
    if (std::isnan(r) || std::isinf(r))
        throw std::runtime_error{
            "power result is too large or not defined in real numbers"
        };
    return visitor.get_runtime().get_number(r);
}

bs_obj::bs_obj_ptr bs_number::mod(interpreter &visitor, const bs_obj_ptr &rhs) const {
    const bs_number *rhs_casted = as_number(rhs);
    if (!rhs_casted) return bs_obj::mod(visitor, rhs);
    if (rhs_casted->value_ == 0) throw std::runtime_error{"division by zero"};
    if (!(std::abs(value_ - std::round(value_)) < eps) || !(
            std::abs(rhs_casted->value_ - std::round(rhs_casted->value_)) < eps))
        throw std::runtime_error{"modulo requires integer operands"};
    return visitor.get_runtime().get_number(static_cast<double>(as_int() % rhs_casted->as_int()));
}

bs_obj::bs_obj_ptr bs_number::fact(interpreter &visitor) const {
    if (!std::isfinite(value_)) throw std::runtime_error{"factorial is not defined for infinite values"};
    if (value_ < 0 && std::abs(value_ - std::round(value_)) < eps)
        throw std::runtime_error{
            "factorial is not defined for negative integers"
        };
    const double r = factorial(value_);
    if (!std::isfinite(r)) throw std::runtime_error{"factorial result is too large"};
    return visitor.get_runtime().get_number(r);
}

bs_obj::bs_obj_ptr bs_number::neg(interpreter &visitor) const {
    return visitor.get_runtime().get_number(-value_);
}

bs_obj::bs_obj_ptr bs_number::pos(interpreter &visitor) const {
    return visitor.get_runtime().get_number(value_);
}

bs_obj::bs_obj_ptr bs_number::eq(interpreter &visitor, const bs_obj_ptr &rhs) const {
    const bs_number *rhs_casted = as_number(rhs);
    if (!rhs_casted) return visitor.get_runtime().false_obj();
    return std::abs(value_ - rhs_casted->value_) < eps ? visitor.get_runtime().true_obj() : visitor.get_runtime().false_obj();
}

bs_obj::bs_obj_ptr bs_number::neq(interpreter &visitor, const bs_obj_ptr &rhs) const {
    const bs_number *rhs_casted = as_number(rhs);
    if (!rhs_casted) return visitor.get_runtime().true_obj();
    return std::abs(value_ - rhs_casted->value_) >= eps ? visitor.get_runtime().true_obj() : visitor.get_runtime().false_obj();
}

bs_obj::bs_obj_ptr bs_number::lt(interpreter &visitor, const bs_obj_ptr &rhs) const {
    const bs_number *rhs_casted = as_number(rhs);
    if (!rhs_casted) return bs_obj::lt(visitor, rhs);
    return value_ < rhs_casted->value_ ? visitor.get_runtime().true_obj() : visitor.get_runtime().false_obj();
}

bs_obj::bs_obj_ptr bs_number::gt(interpreter &visitor, const bs_obj_ptr &rhs) const {
    const bs_number *rhs_casted = as_number(rhs);
    if (!rhs_casted) return bs_obj::gt(visitor, rhs);
    return value_ > rhs_casted->value_ ? visitor.get_runtime().true_obj() : visitor.get_runtime().false_obj();
}

bs_obj::bs_obj_ptr bs_number::le(interpreter &visitor, const bs_obj_ptr &rhs) const {
    const bs_number *rhs_casted = as_number(rhs);
    if (!rhs_casted) return bs_obj::le(visitor, rhs);
    return value_ <= rhs_casted->value_ ? visitor.get_runtime().true_obj() : visitor.get_runtime().false_obj();
}

bs_obj::bs_obj_ptr bs_number::ge(interpreter &visitor, const bs_obj_ptr &rhs) const {
    const bs_number *rhs_casted = as_number(rhs);
    if (!rhs_casted) return bs_obj::ge(visitor, rhs);
    return value_ >= rhs_casted->value_ ? visitor.get_runtime().true_obj() : visitor.get_runtime().false_obj();
}

bool bs_number::to_boolean() const {
    return std::abs(value_) > eps;
}
