//
// Created by semyo on 02.06.2026.
//

#ifndef CPPYTHON_OBJECTVALUE_H
#define CPPYTHON_OBJECTVALUE_H
#include "ReprMixin.h"
#include "Value.h"

class ObjectValue : public ReprMixin {
public:

    [[nodiscard]] virtual Value add(const Value& other) const {
        throw std::runtime_error("Not supported operation for this type");
    }

    [[nodiscard]] virtual Value sub(const Value& other) const {
        throw std::runtime_error("Not supported operation for this type");
    }

    [[nodiscard]] virtual Value multiply(const Value& other) const {
        throw std::runtime_error("Not supported operation for this type");
    }

    [[nodiscard]] virtual Value mod(const Value& other) const {
        throw std::runtime_error("Not supported operation for this type");
    }

    [[nodiscard]] virtual Value rmod(const Value& other) const {
        throw std::runtime_error("Not supported operation for this type");
    }

    virtual Value iadd(const Value& other) {
        throw std::runtime_error(
            "Not supported operation for this type"
        );
    }

    virtual Value imul(const Value& other) {
        throw std::runtime_error(
            "Not supported operation for this type"
        );
    }

    virtual Value ior(const Value& other) {
        throw std::runtime_error(
            "Not supported operation for this type"
        );
    }

    virtual Value iand(const Value& other) {
        throw std::runtime_error(
            "Not supported operation for this type"
        );
    }

    virtual Value isub(const Value& other) {
        throw std::runtime_error(
            "Not supported operation for this type"
        );
    }

    virtual Value ixor(const Value& other) {
        throw std::runtime_error(
            "Not supported operation for this type"
        );
    }

    [[nodiscard]] virtual Value rmul(const Value& other) const {
        throw std::runtime_error("Not supported operation for this type");
    }

    [[nodiscard]] virtual bool contains(const Value& value) const {
        throw std::runtime_error("Not supported operation for this type");
    }

    [[nodiscard]] virtual Value slice(
        const std::optional<Value>& start,
        const std::optional<Value>& stop,
        const std::optional<Value>& step) const {

        throw std::runtime_error("Object is not sliceable");
    }

    [[nodiscard]] virtual bool equal(const Value& other) const {
        throw std::runtime_error("Not supported operation for this type");
    }

    [[nodiscard]] virtual bool notEqual(const Value& other) const {
        throw std::runtime_error("Not supported operation for this type");
    }

    [[nodiscard]] virtual bool lessOrEqual(const Value& other) const {
        throw std::runtime_error("Not supported operation for this type");
    }

    [[nodiscard]] virtual bool less(const Value& other) const {
        throw std::runtime_error("Not supported operation for this type");
    }

    [[nodiscard]] virtual bool greaterOrEqual(const Value& other) const {
        throw std::runtime_error("Not supported operation for this type");
    }

    [[nodiscard]] virtual bool greater(const Value& other) const {
        throw std::runtime_error("Not supported operation for this type");
    }

    [[nodiscard]] virtual Value bitOr(const Value& other) const {
        throw std::runtime_error("Not supported operation for this type");
    }

    [[nodiscard]] virtual Value bitAnd(const Value& other) const {
        throw std::runtime_error("Not supported operation for this type");
    }

    [[nodiscard]] virtual Value bitXor(const Value& other) const {
        throw std::runtime_error("Not supported operation for this type");
    }

    [[nodiscard]] virtual Value ror(const Value& other) const {
        throw std::runtime_error("Not supported operation for this type");
    }

    [[nodiscard]] virtual Value rand(const Value& other) const {
        throw std::runtime_error("Not supported operation for this type");
    }

    [[nodiscard]] virtual Value rsub(const Value& other) const {
        throw std::runtime_error("Not supported operation for this type");
    }

    [[nodiscard]] virtual Value rxor(const Value& other) const {
        throw std::runtime_error("Not supported operation for this type");
    }

    [[nodiscard]] virtual std::size_t hash() const {
        throw std::runtime_error("Not supported operation for this type");
    }

    [[nodiscard]] virtual bool contains(Value&) const {
        throw std::runtime_error("Not supported operation for this type");
    }

    ~ObjectValue() override = default;
};
#endif //CPPYTHON_OBJECTVALUE_H