//
// Created by semyo on 22.05.2026.
//

#ifndef CPPYTHON_ITERATORVALUE_H
#define CPPYTHON_ITERATORVALUE_H

#include "ReprMixin.h"
class Value;

class IteratorValue : public ReprMixin {
public:
    ~IteratorValue() override = default;

    virtual Value next() = 0;

    [[nodiscard]] virtual bool hasNext() const = 0;

    [[nodiscard]] virtual QString getTypeName() const = 0;

    [[nodiscard]] QString toString() const override {

        QString addr = QString("0x%1")
        .arg(reinterpret_cast<quintptr>(this), 0, 16);

        return QString("<%1 object at %2>")
                .arg(getTypeName(), addr);
    }

    [[nodiscard]] QString repr() const override {
        return toString();
    }
};
#endif //CPPYTHON_ITERATORVALUE_H