//
// Created by bibib on 2026/4/5.
//

#include "bs_string.h"
#include "../boolean/bs_boolean.h"
#include "../number/bs_number.h"
#include "../../interpreter/interpreter.h"

bs_string::bs_string(std::string value) : str_(std::move(value)) {
}

bs_obj::bs_obj_ptr bs_string::copy() const {
    return const_cast<bs_string *>(this)->shared_from_this();
}

std::string bs_string::type_name() const {
    return "String";
}

std::string bs_string::to_string() const {
    return str_;
}

bool bs_string::to_boolean() const {
    return !str_.empty();
}

bs_obj::bs_obj_ptr bs_string::add(interpreter &visitor, const bs_obj_ptr &rhs) const {
    if (const auto rhs_casted = dynamic_cast<const bs_string *>(rhs.get())) {
        if (str_.size() > str_.max_size() - rhs_casted->str_.size())
            throw std::runtime_error{"string concatenation result is too large"};

        return visitor.get_runtime().get_string(str_ + rhs_casted->str_);
    }

    return bs_obj::add(visitor, rhs);
}

bs_obj::bs_obj_ptr bs_string::mul(interpreter &visitor, const bs_obj_ptr &rhs) const {
    if (const auto rhs_casted = dynamic_cast<const bs_number *>(rhs.get())) {
        const double val = rhs_casted->value();

        if (!bs_number::is_int(val))
            throw std::runtime_error{"string repeat count must be an integer, got " + rhs_casted->to_string()};

        if (val < 0)
            throw std::runtime_error{"string repeat count must be non-negative, got " + rhs_casted->to_string()};

        if (val > static_cast<double>(std::numeric_limits<size_t>::max()))
            throw std::runtime_error{"string repeat count is too large, got " + rhs_casted->to_string()};


        const auto times = static_cast<size_t>(val);

        if (times == 0)
            return visitor.get_runtime().get_string("");


        if (!str_.empty() && times > str_.max_size() / str_.size())
            throw std::runtime_error{"string repeat result is too large"};


        std::string s;
        s.reserve(str_.size() * times);

        for (size_t i = 0; i < times; ++i)
            s += str_;


        return visitor.get_runtime().get_string(s);
    }

    return bs_obj::mul(visitor, rhs);
}

bs_obj::bs_obj_ptr bs_string::eq(interpreter &visitor, const bs_obj_ptr &rhs) const {
    if (const auto rhs_casted = dynamic_cast<const bs_string *>(rhs.get()))
        return (str_ == rhs_casted->str_) ? visitor.get_runtime().true_obj() : visitor.get_runtime().false_obj();
    return visitor.get_runtime().false_obj();
}

bs_obj::bs_obj_ptr bs_string::subscript(interpreter &visitor, const bs_obj_ptr &index) const {
    if (const auto idx_number = dynamic_cast<const bs_number *>(index.get())) {
        const double value = idx_number->value();
        if (!bs_number::is_int(value))
            throw std::runtime_error{"string index must be an integer, got " + idx_number->to_string()};

        int idx = static_cast<int>(value);
        if (idx < 0) idx = static_cast<int>(str_.size()) + idx;

        if (idx < 0 || static_cast<size_t>(idx) >= str_.size())
            throw std::runtime_error{
                "string index out of range: index is " + std::to_string(idx) +
                ", but length is " + std::to_string(str_.size())
            };

        return visitor.get_runtime().get_string(std::string(1, str_[idx]));
    }
    throw std::runtime_error{"string index must be a number, got " + index->type_name()};
}

size_t bs_string::len() const {
    return str_.size();
}
