//
// Created by semyo on 17.06.2026.
//

#ifndef CPPYTHON_FROZENSETVALUE_H
#define CPPYTHON_FROZENSETVALUE_H
#include <qset.h>

#include "ObjectValue.h"

class FrozenSetValue : public ObjectValue {

    QSet<Value> elements;

public:

    FrozenSetValue() = default;

    explicit FrozenSetValue(const QSet<Value>& elements)
        : elements(elements)
    {}

    explicit FrozenSetValue(QSet<Value>&& elements)
        : elements(std::move(elements))
    {}

    [[nodiscard]] QString repr() const override;

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] std::size_t len() const;

    [[nodiscard]] bool contains(const Value& value) const override;

    [[nodiscard]] const QSet<Value>& getElements() const {
        return elements;
    }
};
#endif //CPPYTHON_FROZENSETVALUE_H