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

    [[nodiscard]] virtual Value multiply(const Value& other) const {
        throw std::runtime_error("Not supported operation for this type");
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

    ~ObjectValue() override = default;
};
#endif //CPPYTHON_OBJECTVALUE_H