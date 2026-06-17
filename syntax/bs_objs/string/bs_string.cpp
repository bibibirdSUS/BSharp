//
// Created by bibibird on 2026/4/5.
//

#include "bs_string.h"
#include <functional>

#include "../boolean/bs_boolean.h"
#include "../number/bs_number.h"
#include "../../interpreter/interpreter.h"

namespace {
    size_t get_utf8_length(const std::string &str) {
        size_t char_count = 0;

        for (const char i: str)
            if ((static_cast<unsigned char>(i) & 0xC0) != 0x80)
                char_count++;

        return char_count;
    }

    size_t get_utf8_byte_pos(const std::string &str, const size_t target_char_idx) {
        size_t char_count = 0;
        for (size_t byte_idx = 0; byte_idx < str.size(); ++byte_idx)
            if ((static_cast<unsigned char>(str[byte_idx]) & 0xC0) != 0x80) {
                if (char_count == target_char_idx) return byte_idx;
                char_count++;
            }

        return std::string::npos;
    }
}

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

size_t bs_string::hash() const {
    return std::hash<std::string>{}(str_);
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

        if (!rhs_casted->is_int())
            throw std::runtime_error{"string repeat count must be an integer, got " + rhs_casted->to_string()};

        const double val = rhs_casted->value();

        if (val < 0)
            throw std::runtime_error{"string repeat count must be non-negative, got " + rhs_casted->to_string()};

        if (val > static_cast<double>(std::numeric_limits<size_t>::max()))
            throw std::runtime_error{"string repeat count is too large, got " + rhs_casted->to_string()};


        const auto times = rhs_casted->as_int();

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
        return str_ == rhs_casted->str_ ? visitor.get_runtime().true_obj() : visitor.get_runtime().false_obj();
    return visitor.get_runtime().false_obj();
}

bs_obj::bs_obj_ptr bs_string::subscript(interpreter &visitor, const bs_obj_ptr &index) const {
    if (const auto idx_number = dynamic_cast<const bs_number *>(index.get())) {
        if (!idx_number->is_int())
            throw std::runtime_error{"string index must be an integer, got " + idx_number->to_string()};

        int64_t idx = idx_number->as_int();
        const size_t utf8_len = len();

        if (idx < 0) idx = static_cast<int64_t>(utf8_len) + idx;

        if (idx < 0 || static_cast<size_t>(idx) >= utf8_len)
            throw std::runtime_error{
                "string index out of range: index is " + std::to_string(idx) +
                ", but length is " + std::to_string(str_.size())
            };

        const size_t byte_start = get_utf8_byte_pos(str_, static_cast<size_t>(idx));
        const size_t byte_end = static_cast<size_t>(idx) + 1 < utf8_len
                                    ? get_utf8_byte_pos(str_, static_cast<size_t>(idx) + 1)
                                    : str_.size();

        const size_t char_bytes = byte_end - byte_start;
        return visitor.get_runtime().get_string(str_.substr(byte_start, char_bytes));
    }
    throw std::runtime_error{"string index must be a number, got " + index->type_name()};
}

bs_obj::bs_obj_ptr bs_string::slice(interpreter &visitor, const int64_t start, const int64_t end, const int64_t step) const {
    const size_t utf8_len = len();
    if (start == end) return visitor.get_runtime().get_string("");

    // Cache byte positions for O(N) slicing
    std::vector<size_t> byte_positions;
    byte_positions.reserve(utf8_len + 1);
    for (size_t byte_idx = 0; byte_idx < str_.size(); ++byte_idx)
        if ((static_cast<unsigned char>(str_[byte_idx]) & 0xC0) != 0x80)
            byte_positions.push_back(byte_idx);
    byte_positions.push_back(str_.size());

    if (step == 1) {
        if (start >= end) return visitor.get_runtime().get_string("");
        const size_t b_start = byte_positions[static_cast<size_t>(start)];
        const size_t b_end = byte_positions[static_cast<size_t>(end)];
        return visitor.get_runtime().get_string(str_.substr(b_start, b_end - b_start));
    }

    std::string result;
    if (step > 0) {
        for (int64_t i = start; i < end; i += step) {
            const size_t b_start = byte_positions[static_cast<size_t>(i)];
            const size_t b_end = byte_positions[static_cast<size_t>(i) + 1];
            result += str_.substr(b_start, b_end - b_start);
        }
    } else {
        for (int64_t i = start; i > end; i += step) {
            const size_t b_start = byte_positions[static_cast<size_t>(i)];
            const size_t b_end = byte_positions[static_cast<size_t>(i) + 1];
            result += str_.substr(b_start, b_end - b_start);
        }
    }
    return visitor.get_runtime().get_string(result);
}

size_t bs_string::len() const {
    return get_utf8_length(str_);
}
