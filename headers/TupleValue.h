//
// Created by semyo on 17.05.2026.
//

#ifndef CPPYTHON_TUPLEVALUE_H
#define CPPYTHON_TUPLEVALUE_H
#include <vector>

#include "ReprMixin.h"

class Value;

class TupleValue : public ReprMixin {
public:
    std::vector<Value> items;

    TupleValue() = default;

    explicit TupleValue(const std::vector<Value>& items);

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] QString repr() const override;

    [[nodiscard]] Value getItem(const Value& index) const;

    [[nodiscard]] Value count(const Value& value) const;
};

#endif //CPPYTHON_TUPLEVALUE_H