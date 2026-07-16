//
// Created by semyo on 12.07.2026.
//

#ifndef CPPYTHON_RANGEITERATOR_H
#define CPPYTHON_RANGEITERATOR_H
#include <memory>
#include "IteratorValue.h"
#include "Value.h"

class RangeValue;

class RangeIterator : public IteratorValue {
public:
    Value::BigInt current;
    Value::BigInt stop;
    Value::BigInt step;

    explicit RangeIterator(const std::shared_ptr<RangeValue>& range);

    Value next() override;
    [[nodiscard]] bool hasNext() const override;
    [[nodiscard]] QString getTypeName() const override;
};
#endif