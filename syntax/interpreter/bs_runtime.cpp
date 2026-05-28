//
// Created by bibib on 2026/4/5.
//

#include "bs_runtime.h"

#include "../bs_objs/boolean/bs_boolean.h"
#include "../bs_objs/bs_list/bs_list.h"
#include "../bs_objs/function/bs_function.h"
#include "../bs_objs/null/bs_null.h"

bs_runtime::bs_runtime() : bs_true(std::make_shared<bs_boolean>(true)), bs_false(std::make_shared<bs_boolean>(false)),
                           bs_null(std::make_shared<::bs_null>()) {
    integer_cache.reserve(INTEGER_CACHE_MAX - INTEGER_CACHE_MIN + 1);
    for (int i = INTEGER_CACHE_MIN; i <= INTEGER_CACHE_MAX; ++i)
        integer_cache.push_back(std::make_shared<bs_number>(i));
}

bs_obj_ptr bs_runtime::true_obj() const {
    return bs_true;
}

bs_obj_ptr bs_runtime::false_obj() const {
    return bs_false;
}

bs_obj_ptr bs_runtime::null_obj() const {
    return bs_null;
}

bs_obj_ptr bs_runtime::get_string(const std::string &value) {
    if (const auto it = string_pool.find(value); it != string_pool.end()) {
        if (auto shared = it->second.lock())
            return shared;
        string_pool.erase(it);
    }

    auto obj = std::make_shared<bs_string>(value);
    string_pool[value] = obj;
    return obj;
}

bs_obj_ptr bs_runtime::get_number(double value) const {
    if (bs_number::is_int(value) && value >= INTEGER_CACHE_MIN && value <= INTEGER_CACHE_MAX)
        return integer_cache[static_cast<int>(value) - INTEGER_CACHE_MIN];
    return std::make_shared<bs_number>(value);
}

bs_obj_ptr bs_runtime::get_function(const std::string &name, const std::vector<parameter> &params,
                                    const node_ptr &body,
                                    const context_ptr &closure) {
    return std::make_shared<bs_function>(name, params, body->copy(), closure);
}

bs_obj_ptr bs_runtime::get_list(std::vector<bs_obj_ptr> elements) {
    return std::make_shared<bs_list>(std::move(elements));
}
