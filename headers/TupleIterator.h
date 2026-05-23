//
// Created by semyo on 18.05.2026.
//

#ifndef CPPYTHON_TUPLEITERATORVALUE_H
#define CPPYTHON_TUPLEITERATORVALUE_H
#include <memory>

#include "IteratorValue.h"

class TupleValue;

class TupleIterator : public IteratorValue {
public:

    std::shared_ptr<TupleValue> tuple;
    std::size_t index = 0;

    explicit TupleIterator(std::shared_ptr<TupleValue> tuple)
    : tuple(std::move(tuple)) {}

    Value next() override;

    [[nodiscard]] bool hasNext() const override;

    [[nodiscard]] QString getTypeName() const override;
};
#endif //CPPYTHON_TUPLEITERATORVALUE_H