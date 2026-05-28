//
// Created by bibib on 2026/3/28.
//

#ifndef BSHARP_BS_FUNCTION_H
#define BSHARP_BS_FUNCTION_H

#include <utility>
#include <vector>

#include "../bs_obj.h"
#include "../../parser/nodes.h"

#include "../../interpreter/context.h"

class bs_function : public bs_obj {
public:
    typedef context::context_ptr context_ptr;

    bs_function(std::string name, std::vector<parameter> params, node_ptr body, context_ptr closure)
        : name_(std::move(name)),
          params_(std::move(params)),
          body_(std::move(body)),
          closure_(std::move(closure)) {
    }

    [[nodiscard]] bs_obj_ptr copy() const override;

    [[nodiscard]] std::string type_name() const override;

    [[nodiscard]] bs_obj_ptr call(interpreter &visitor, const std::vector<bs_obj_ptr> &args) const override;

    bool is_variadic() const {
        return !params_.empty() && params_.back().is_variadic;
    }

private:
    std::string name_;
    std::vector<parameter> params_;
    node_ptr body_;
    context_ptr closure_;
};


#endif //BSHARP_BS_FUNCTION_H
