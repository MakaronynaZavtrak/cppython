//
// Created by semyo on 17.06.2026.
//

#ifndef CPPYTHON_FROZENSETVALUE_H
#define CPPYTHON_FROZENSETVALUE_H
#include <qset.h>

#include "ObjectValue.h"

class FrozenSetValue : public ObjectValue {

    QSet<Value> elements;
    QList<Value> order;

public:

    FrozenSetValue() = default;

    explicit FrozenSetValue(const QSet<Value>& elements)
    : elements(elements),
      order(elements.values())
    {}


    explicit FrozenSetValue(QSet<Value>&& elements)
    : elements(std::move(elements)),
      order(this->elements.values())
    {}

    [[nodiscard]] QString repr() const override;

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] std::size_t len() const;

    [[nodiscard]] bool contains(const Value& value) const override;

    [[nodiscard]] const QSet<Value>& getElements() const;

    [[nodiscard]] const QList<Value>& getOrder() const;

    [[nodiscard]] Value unionSet(const std::vector<Value>& others) const;

    [[nodiscard]] Value intersection(const std::vector<Value>& others) const;

    [[nodiscard]] Value difference(const std::vector<Value>& others) const;

    [[nodiscard]] Value symmetricDifference(const Value& other) const;

    [[nodiscard]] Value bitOr(const Value& other) const;

    [[nodiscard]] Value bitAnd(const Value& other) const;
};
#endif //CPPYTHON_FROZENSETVALUE_H