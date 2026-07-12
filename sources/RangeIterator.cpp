//
// Created by semyo on 12.07.2026.
//

#include "RangeIterator.h"
#include "RangeValue.h"
#include "../exception/StopIterationException.h"

RangeIterator::RangeIterator(const std::shared_ptr<RangeValue>& range)
    : current(range->start), stop(range->stop), step(range->step) {}

bool RangeIterator::hasNext() const {
    return step > 0 ? current < stop : current > stop;
}

Value RangeIterator::next() {
    if (!hasNext()) {
        throw StopIterationException();
    }
    Value::BigInt v = current;
    current += step;
    return Value(v);
}

QString RangeIterator::getTypeName() const {
    return "range_iterator";
}