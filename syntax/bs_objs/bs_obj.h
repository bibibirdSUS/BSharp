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
        throw std::runtime_error{"Class '" + type_name() + "' can't be converted to boolean"};
    }

    [[nodiscard]] virtual bs_obj_ptr call(interpreter &visitor, const std::vector<bs_obj_ptr> &args) const {
        throw std::runtime_error{"Class '" + type_name() + "' is not callable"};
    }

    [[nodiscard]] virtual bs_obj_ptr neg() const {
        throw std::runtime_error{"Class '" + type_name() + "' doesn't support '-'"};
    }

    [[nodiscard]] virtual bs_obj_ptr pos() const {
        throw std::runtime_error{"Class '" + type_name() + "' doesn't support '+'"};
    }

    [[nodiscard]] virtual bs_obj_ptr add(const bs_obj_ptr &rhs) const {
        throw std::runtime_error{"Class '" + type_name() + "' doesn't support '+' with " + rhs->type_name()};
    }

    [[nodiscard]] virtual bs_obj_ptr sub(const bs_obj_ptr &rhs) const {
        throw std::runtime_error{"Class '" + type_name() + "' doesn't support '-' with " + rhs->type_name()};
    }

    [[nodiscard]] virtual bs_obj_ptr mul(const bs_obj_ptr &rhs) const {
        throw std::runtime_error{"Class '" + type_name() + "' doesn't support '*' with " + rhs->type_name()};
    }

    [[nodiscard]] virtual bs_obj_ptr div(const bs_obj_ptr &rhs) const {
        throw std::runtime_error{"Class '" + type_name() + "' doesn't support '/' with " + rhs->type_name()};
    }

    [[nodiscard]] virtual bs_obj_ptr pow(const bs_obj_ptr &rhs) const {
        throw std::runtime_error{"Class '" + type_name() + "' doesn't support '^' with " + rhs->type_name()};
    }

    [[nodiscard]] virtual bs_obj_ptr mod(const bs_obj_ptr &rhs) const {
        throw std::runtime_error{"Class '" + type_name() + "' doesn't support '%' with " + rhs->type_name()};
    }

    [[nodiscard]] virtual bs_obj_ptr fact() const {
        throw std::runtime_error{"Class '" + type_name() + "' doesn't support '!'"};
    }

    [[nodiscard]] virtual bs_obj_ptr eq(const bs_obj_ptr &rhs) const;

    [[nodiscard]] virtual bs_obj_ptr neq(const bs_obj_ptr &rhs) const;

    [[nodiscard]] virtual bs_obj_ptr lt(const bs_obj_ptr &rhs) const {
        throw std::runtime_error{"Class '" + type_name() + "' doesn't support '<' with " + rhs->type_name()};
    }

    [[nodiscard]] virtual bs_obj_ptr gt(const bs_obj_ptr &rhs) const {
        throw std::runtime_error{"Class '" + type_name() + "' doesn't support '>' with " + rhs->type_name()};
    }

    [[nodiscard]] virtual bs_obj_ptr le(const bs_obj_ptr &rhs) const {
        throw std::runtime_error{"Class '" + type_name() + "' doesn't support '<=' with " + rhs->type_name()};
    }

    [[nodiscard]] virtual bs_obj_ptr ge(const bs_obj_ptr &rhs) const {
        throw std::runtime_error{"Class '" + type_name() + "' doesn't support '>=' with " + rhs->type_name()};
    }

    [[nodiscard]] virtual bs_obj_ptr call(const std::vector<bs_obj_ptr> &args) const {
        throw std::runtime_error{"Class '" + type_name() + "' is not callable"};
    }

    [[nodiscard]] virtual bs_obj_ptr subscript(const bs_obj_ptr &index) const {
        throw std::runtime_error{"Class '" + type_name() + "' does not support indexing with '[]'"};
    }

    virtual void set_subscript(const bs_obj_ptr &index, const bs_obj_ptr &value) {
        throw std::runtime_error{"Class '" + type_name() + "' does not support index assignment"};
    }
};


#endif //BSHARP_BS_OBJ_H
