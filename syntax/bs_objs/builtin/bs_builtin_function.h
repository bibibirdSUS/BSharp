//
// Created by bibibird on 2026/5/18.
//

#ifndef BSHARP_BUILTIN_FUNCTION_H
#define BSHARP_BUILTIN_FUNCTION_H
#include <functional>

#include "../bs_obj.h"

class bs_builtin_function : public bs_obj {
    typedef std::function<bs_obj_ptr(interpreter &, const std::vector<bs_obj_ptr> &)> native_fn;

public:
    bs_builtin_function(std::string name,
                        std::function<bs_obj_ptr(interpreter &, const std::vector<bs_obj_ptr> &)> cpp_fn);

    [[nodiscard]] bs_obj_ptr copy() const override;

    [[nodiscard]] std::string type_name() const override;

    [[nodiscard]] bs_obj_ptr call(interpreter &visitor, const std::vector<bs_obj_ptr> &args) const override;

private:
    native_fn func_;
    std::string name_;
};


#endif //BSHARP_BUILTIN_FUNCTION_H
