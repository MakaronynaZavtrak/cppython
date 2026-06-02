//
// Created by semyo on 17.05.2026.
//

#ifndef CPPYTHON_TUPLEVALUE_H
#define CPPYTHON_TUPLEVALUE_H
#include <vector>

#include "ObjectValue.h"
#include "Value.h"

class TupleValue : public ObjectValue {
public:
    std::vector<Value> items;

    TupleValue() = default;

    explicit TupleValue(const std::vector<Value>& items);

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] QString repr() const override;

    [[nodiscard]] Value getItem(const Value& index) const;

    [[nodiscard]] Value count(const Value& value) const;

    [[nodiscard]] Value index(
    const Value& value,
    const std::optional<Value>& start = std::nullopt,
    const std::optional<Value>& end = std::nullopt) const;

    [[nodiscard]] std::size_t len() const;

    [[nodiscard]] bool equal(const Value &other) const override;

    [[nodiscard]] bool notEqual(const Value &other) const override;

    [[nodiscard]] bool lessOrEqual(const Value &other) const override;

    [[nodiscard]] bool less(const Value &other) const override;

    [[nodiscard]] bool greaterOrEqual(const Value &other) const override;

    [[nodiscard]] bool greater(const Value &other) const override;
};

#endif //CPPYTHON_TUPLEVALUE_H