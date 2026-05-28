//
// Created by bibib on 2026/2/28.
//

#ifndef BSHARP_BS_OBJ_H
#define BSHARP_BS_OBJ_H
#include <memory>
#include <string>
#include <vector>


class interpreter;
class context;

class bs_obj : public std::enable_shared_from_this<bs_obj> {
public:
    typedef std::shared_ptr<bs_obj> bs_obj_ptr;

    virtual ~bs_obj() = default;

    [[nodiscard]] virtual bs_obj_ptr copy() const = 0;

    [[nodiscard]] virtual std::string type_name() const = 0;

    [[nodiscard]] virtual std::string to_string() const {
        return "<" + type_name() + ">";
    }

    [[nodiscard]] virtual bool to_boolean() const {
        throw std::runtime_error{"cannot convert " + type_name() + " to Boolean"};
    }

    [[nodiscard]] virtual bs_obj_ptr call(interpreter &visitor, const std::vector<bs_obj_ptr> &args) const {
        throw std::runtime_error{type_name() + " is not callable"};
    }

    // --- 數學與二元運算子 ---
    [[nodiscard]] virtual bs_obj_ptr neg(interpreter &visitor) const {
        throw std::runtime_error{type_name() + " does not support unary '-'"};
    }

    [[nodiscard]] virtual bs_obj_ptr pos(interpreter &visitor) const {
        throw std::runtime_error{type_name() + " does not support unary '+'"};
    }

    [[nodiscard]] virtual bs_obj_ptr fact(interpreter &visitor) const {
        throw std::runtime_error{type_name() + " does not support '!'"};
    }

    [[nodiscard]] virtual bs_obj_ptr add(interpreter &visitor, const bs_obj_ptr &rhs) const {
        throw std::runtime_error{type_name() + " does not support '+' with " + rhs->type_name()};
    }

    [[nodiscard]] virtual bs_obj_ptr sub(interpreter &visitor, const bs_obj_ptr &rhs) const {
        throw std::runtime_error{type_name() + " does not support '-' with " + rhs->type_name()};
    }

    [[nodiscard]] virtual bs_obj_ptr mul(interpreter &visitor, const bs_obj_ptr &rhs) const {
        throw std::runtime_error{type_name() + " does not support '*' with " + rhs->type_name()};
    }

    [[nodiscard]] virtual bs_obj_ptr div(interpreter &visitor, const bs_obj_ptr &rhs) const {
        throw std::runtime_error{type_name() + " does not support '/' with " + rhs->type_name()};
    }

    [[nodiscard]] virtual bs_obj_ptr pow(interpreter &visitor, const bs_obj_ptr &rhs) const {
        throw std::runtime_error{type_name() + " does not support '^' with " + rhs->type_name()};
    }

    [[nodiscard]] virtual bs_obj_ptr mod(interpreter &visitor, const bs_obj_ptr &rhs) const {
        throw std::runtime_error{type_name() + " does not support '%' with " + rhs->type_name()};
    }

    // --- 比較運算子 ---
    [[nodiscard]] virtual bs_obj_ptr eq(interpreter &visitor, const bs_obj_ptr &rhs) const;

    [[nodiscard]] virtual bs_obj_ptr neq(interpreter &visitor, const bs_obj_ptr &rhs) const;

    [[nodiscard]] virtual bs_obj_ptr lt(interpreter &visitor, const bs_obj_ptr &rhs) const {
        throw std::runtime_error{type_name() + " does not support '<' with " + rhs->type_name()};
    }

    [[nodiscard]] virtual bs_obj_ptr gt(interpreter &visitor, const bs_obj_ptr &rhs) const {
        throw std::runtime_error{type_name() + " does not support '>' with " + rhs->type_name()};
    }

    [[nodiscard]] virtual bs_obj_ptr le(interpreter &visitor, const bs_obj_ptr &rhs) const {
        throw std::runtime_error{type_name() + " does not support '<=' with " + rhs->type_name()};
    }

    [[nodiscard]] virtual bs_obj_ptr ge(interpreter &visitor, const bs_obj_ptr &rhs) const {
        throw std::runtime_error{type_name() + " does not support '>=' with " + rhs->type_name()};
    }

    [[nodiscard]] virtual bs_obj_ptr call(const std::vector<bs_obj_ptr> &args) const {
        throw std::runtime_error{type_name() + " is not callable"};
    }

    [[nodiscard]] virtual bs_obj_ptr subscript(interpreter &visitor, const bs_obj_ptr &index) const {
        throw std::runtime_error{type_name() + " does not support indexing"};
    }

    virtual void set_subscript(interpreter &visitor, const bs_obj_ptr &index, const bs_obj_ptr &value) {
        throw std::runtime_error{type_name() + " does not support index assignment"};
    }

    [[nodiscard]] virtual size_t len() const {
        throw std::runtime_error{type_name() + " does not support len()"};
    }
};


#endif //BSHARP_BS_OBJ_H
