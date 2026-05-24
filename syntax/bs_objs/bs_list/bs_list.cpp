//
// Created by bibib on 2026/5/22.
//

#include "bs_list.h"
#include <sstream>

#include "../number/bs_number.h"
#include "../../interpreter/interpreter.h"

bs_list::bs_list(std::vector<bs_obj_ptr> elements) : elements_(std::move(elements)) {
}

bs_obj::bs_obj_ptr bs_list::copy() const {
    return std::make_shared<bs_list>(elements_);
}

std::string bs_list::type_name() const {
    return "List";
}

bs_obj::bs_obj_ptr bs_list::subscript(interpreter &visitor, const bs_obj_ptr &index) const {
    const auto idx_number = dynamic_cast<const bs_number *>(index.get());
    if (!idx_number)
        throw std::runtime_error{"List indices must be numbers, not '" + index->type_name() + "'"};
    const double value = idx_number->value();
    if (!bs_number::is_int(value))
        throw std::runtime_error{"List indices must be integers"};

    int idx = static_cast<int>(value);

    if (idx < 0)
        idx = static_cast<int>(elements_.size()) + idx;

    if (idx < 0 || static_cast<size_t>(idx) >= elements_.size())
        throw std::runtime_error{
            "List index out of range: index is " + std::to_string(idx) +
            ", but size is " + std::to_string(size())
        };

    return elements_[static_cast<size_t>(idx)];
}

void bs_list::set_subscript(interpreter &visitor, const bs_obj_ptr &index, const bs_obj_ptr &value) {
    const auto idx_number = dynamic_cast<const bs_number *>(index.get());
    if (!idx_number)
        throw std::runtime_error{"List indices must be numbers, not '" + index->type_name() + "'"};
    const double val = idx_number->value();
    if (!bs_number::is_int(val))
        throw std::runtime_error{"List indices must be integers"};

    int idx = static_cast<int>(val);
    if (idx < 0)
        idx = static_cast<int>(elements_.size()) + idx;

    if (idx < 0 || static_cast<size_t>(idx) >= elements_.size())
        throw std::runtime_error{"List index out of range"};

    elements_[static_cast<size_t>(idx)] = value;
}

std::string bs_list::to_string() const {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < elements_.size(); ++i) {
        ss << elements_[i]->to_string();
        if (i != elements_.size() - 1) ss << ", ";
    }
    ss << "]";
    return ss.str();
}

bool bs_list::to_boolean() const {
    return !elements_.empty();
}

bs_obj::bs_obj_ptr bs_list::eq(interpreter &visitor, const bs_obj_ptr &rhs) const {
    const auto rhs_casted = dynamic_cast<const bs_list *>(rhs.get());
    if (!rhs_casted) return visitor.get_runtime().false_obj();
    if (elements_.size() != rhs_casted->elements_.size()) return visitor.get_runtime().false_obj();

    for (size_t i = 0; i < elements_.size(); ++i) {
        if (!elements_[i]->eq(visitor, rhs_casted->elements_[i])->to_boolean())
            return visitor.get_runtime().false_obj();
    }
    return visitor.get_runtime().true_obj();
}

void bs_list::append(bs_obj_ptr obj) {
    elements_.push_back(std::move(obj));
}

size_t bs_list::size() const {
    return elements_.size();
}

bs_obj::bs_obj_ptr bs_list::get(const size_t idx) {
    return elements_[idx];
}
