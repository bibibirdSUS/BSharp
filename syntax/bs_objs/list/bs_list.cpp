//
// Created by bibibird on 2026/5/22.
//

#include "bs_list.h"

#include <limits>
#include <sstream>
#include <unordered_set>

#include "bs_list_iterator.h"
#include "../../utils.h"
#include "../number/bs_number.h"
#include "../../interpreter/interpreter.h"

namespace {
    thread_local std::unordered_set<const bs_list *> lists_being_hashed;
}

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
        throw std::runtime_error{"list index must be a number, got " + index->type_name()};

    if (!idx_number->is_int())
        throw std::runtime_error{"list index must be an integer, got " + idx_number->to_string()};

    int64_t idx = idx_number->as_int();

    if (idx < 0)
        idx = static_cast<int64_t>(elements_.size()) + idx;

    if (idx < 0 || static_cast<size_t>(idx) >= elements_.size())
        throw std::runtime_error{
            "list index out of range: index is " + std::to_string(idx) +
            ", but size is " + std::to_string(size())
        };

    return elements_[static_cast<size_t>(idx)];
}

void bs_list::set_subscript(interpreter &visitor, const bs_obj_ptr &index, const bs_obj_ptr &value) {
    const auto idx_number = dynamic_cast<const bs_number *>(index.get());
    if (!idx_number)
        throw std::runtime_error{"list index must be a number, got " + index->type_name()};

    if (!idx_number->is_int())
        throw std::runtime_error{"list index must be an integer, got " + idx_number->to_string()};

    int64_t idx = idx_number->as_int();
    if (idx < 0)
        idx = static_cast<int64_t>(elements_.size()) + idx;

    if (idx < 0 || static_cast<size_t>(idx) >= elements_.size())
        throw std::runtime_error{
            "list index out of range: index is " + std::to_string(idx) +
            ", but size is " + std::to_string(size())
        };

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

size_t bs_list::len() const {
    return size();
}

size_t bs_list::hash() const {
    if (lists_being_hashed.contains(this))
        throw std::runtime_error{"cannot hash recursive list"};

    lists_being_hashed.insert(this);

    size_t seed = hash_combine(std::hash<std::string>{}("List"), elements_.size());
    try {
        for (const auto &element: elements_)
            seed = hash_combine(seed, element ? element->hash() : 0);
    } catch (...) {
        lists_being_hashed.erase(this);
        throw;
    }

    lists_being_hashed.erase(this);
    return seed;
}

bs_obj_ptr bs_list::add(interpreter &visitor, const bs_obj_ptr &rhs) const {
    if (const auto rhs_casted = dynamic_cast<const bs_list *>(rhs.get())) {
        std::vector<bs_obj_ptr> concatenated_elements;
        concatenated_elements.reserve(elements_.size() + rhs_casted->elements_.size());

        for (const auto &left: elements_)
            concatenated_elements.push_back(left->copy());
        for (const auto &right: rhs_casted->elements_)
            concatenated_elements.push_back(right->copy());
        return bs_runtime::get_list(std::move(concatenated_elements));
    }
    return bs_obj::add(visitor, rhs);
}

bs_obj_ptr bs_list::mul(interpreter &visitor, const bs_obj_ptr &rhs) const {
    if (const auto rhs_casted = dynamic_cast<const bs_number *>(rhs.get())) {
        const double val = rhs_casted->value();
        if (!bs_number::is_int(val))
            throw std::runtime_error{"list repeat count must be an integer, got " + rhs_casted->to_string()};
        if (val < 0)
            throw std::runtime_error{"list repeat count must be non-negative, got " + rhs_casted->to_string()};
        if (val > static_cast<double>(std::numeric_limits<size_t>::max()))
            throw std::runtime_error{"list repeat count is too large, got " + rhs_casted->to_string()};

        std::vector<bs_obj_ptr> repeated_elements;
        const auto times = static_cast<size_t>(val);
        if (!elements_.empty() && times > std::numeric_limits<size_t>::max() / elements_.size())
            throw std::runtime_error{"list repeat result is too large"};
        repeated_elements.reserve(elements_.size() * times);
        for (size_t i = 0; i < times; ++i)
            for (const auto &element: elements_)
                repeated_elements.push_back(element->copy());
        return bs_runtime::get_list(std::move(repeated_elements));
    }

    return bs_obj::mul(visitor, rhs);
}

bs_obj::bs_obj_ptr bs_list::slice(interpreter &visitor, const int64_t start, const int64_t end,
                                  const int64_t step) const {
    std::vector<bs_obj_ptr> sliced_elements;
    sliced_elements.reserve(std::abs(end - start) / std::abs(step) + 1);

    if (step > 0)
        for (int64_t i = start; i < end; i += step)
            sliced_elements.push_back(elements_[i]);

    else
        for (int64_t i = start; i > end; i += step)
            sliced_elements.push_back(elements_[i]);


    return bs_runtime::get_list(std::move(sliced_elements));
}

void bs_list::append(bs_obj_ptr obj) {
    elements_.push_back(std::move(obj));
}

size_t bs_list::size() const {
    return elements_.size();
}

bs_obj::bs_obj_ptr bs_list::get(const size_t idx) const {
    return elements_[idx];
}

std::unique_ptr<bs_iterator> bs_list::iter() const {
    return std::make_unique<bs_list_iterator>(*this);
}
