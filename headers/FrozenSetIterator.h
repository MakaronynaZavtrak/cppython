//
// Created by semyo on 18.06.2026.
//

#ifndef CPPYTHON_FROZENSETITERATOR_H
#define CPPYTHON_FROZENSETITERATOR_H
#include <memory>

#include "IteratorValue.h"

class FrozenSetValue;

class FrozenSetIterator : public IteratorValue {

public:

    std::shared_ptr<FrozenSetValue> frozenSet;

    int index = 0;

    explicit FrozenSetIterator(
        std::shared_ptr<FrozenSetValue> frozenSet
    )
        : frozenSet(std::move(frozenSet))
    {}

    Value next() override;

    [[nodiscard]] bool hasNext() const override;

    [[nodiscard]] QString getTypeName() const override;
};

#endif //CPPYTHON_FROZENSETITERATOR_H