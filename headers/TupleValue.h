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

    QString toString() const override;

    QString repr() const override;

    Value getItem(const Value& index) const;
};

#endif //CPPYTHON_TUPLEVALUE_H