//
// Created by semyo on 12.05.2026.
//

#ifndef CPPYTHON_LISTVALUE_H
#define CPPYTHON_LISTVALUE_H
#include <vector>

#include "ObjectValue.h"
#include "Value.h"

class ListValue : public ObjectValue {
public:
    std::vector<Value> elements;

    ListValue() = default;

    explicit ListValue(std::vector<Value> elems)
        : elements(std::move(elems)) {}

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] QString repr() const override;

    Value getItem(const Value& index);

    void setItem(const Value& index, const Value& value);

    void append(const Value& value);

    Value pop(const std::optional<Value>& index = std::nullopt);

    std::size_t len() const;

    void extend(const Value& other);

    void insert(const Value& index, const Value& value);

    void remove(const Value& value);

    void clear();

    Value count(const Value& value) const;

    Value index(const Value& value,
                const std::optional<Value>& start = std::nullopt,
                const std::optional<Value>& end = std::nullopt) const;

    void reverse();

    Value copy() const;

    void sort(const std::optional<Value>& key, bool reverse, const std::shared_ptr<Environment>& env);

    [[nodiscard]] bool equal(const Value& other) const override;

    [[nodiscard]] bool notEqual(const Value& value) const override;

    [[nodiscard]] bool lessOrEqual(const Value &other) const override;

    [[nodiscard]] bool less(const Value& value) const override;

    [[nodiscard]] bool greaterOrEqual(const Value &other) const override;

    [[nodiscard]] bool greater(const Value &other) const override;
};
#endif //CPPYTHON_LISTVALUE_H