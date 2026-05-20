//
// Created by bibib on 2026/4/5.
//

#ifndef BSHARP_BS_RUNTIME_H
#define BSHARP_BS_RUNTIME_H
#include "context.h"
#include "../bs_objs/number/bs_number.h"
#include "../bs_objs/string/bs_string.h"


class bs_runtime {
public:
    static constexpr int INTEGER_CACHE_MAX = 255, INTEGER_CACHE_MIN = -256;

    bs_runtime();

    bs_obj_ptr true_obj() const;

    bs_obj_ptr false_obj() const;

    bs_obj_ptr null_obj() const;

    bs_obj_ptr get_string(const std::string &value);

    bs_obj_ptr get_number(double value) const;

private:
    const bs_obj_ptr bs_true;
    const bs_obj_ptr bs_false;
    const bs_obj_ptr bs_null;
    std::unordered_map<std::string, std::weak_ptr<bs_string> > string_pool{};
    std::vector<bs_obj_ptr> integer_cache{};
};


#endif //BSHARP_BS_RUNTIME_H
