//
// Created by semyo on 12.07.2026.
//

#ifndef CPPYTHON_RANGEVALUE_H
#define CPPYTHON_RANGEVALUE_H

#include "ObjectValue.h"
#include "Value.h"

class RangeValue : public ObjectValue {
public:
    Value::BigInt start;
    Value::BigInt stop;
    Value::BigInt step;

    RangeValue(Value::BigInt start, Value::BigInt stop, Value::BigInt step);

    [[nodiscard]] QString toString() const override;
    [[nodiscard]] QString repr() const override;

    [[nodiscard]] Value getItem(const Value& index) const override;

    [[nodiscard]] std::size_t len() const;

    [[nodiscard]] bool equal(const Value& other) const override;
    [[nodiscard]] bool notEqual(const Value& other) const override;
    [[nodiscard]] bool lessOrEqual(const Value& other) const override;
    [[nodiscard]] bool less(const Value& other) const override;
    [[nodiscard]] bool greaterOrEqual(const Value& other) const override;
    [[nodiscard]] bool greater(const Value& other) const override;

    [[nodiscard]] std::size_t hash() const override;

    [[nodiscard]] bool contains(const Value& value) const override;

    [[nodiscard]] Value count(const Value& value) const;

    [[nodiscard]] Value index(const Value& value) const;

    [[nodiscard]] bool toBool() const;
};

#endif //CPPYTHON_RANGEVALUE_H