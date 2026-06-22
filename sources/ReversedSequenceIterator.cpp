//
// Created by semyo on 22.06.2026.
//
#include "ReversedSequenceIterator.h"

#include "CallRuntime.h"
#include "ClassUtils.h"
#include "StopIterationException.h"

ReversedSequenceIterator::ReversedSequenceIterator(const Value& object, const std::ptrdiff_t length)
    : object(object), index(length - 1) {}

Value ReversedSequenceIterator::next() {

    if (!hasNext()) {
        throw StopIterationException();
    }

    Value getItem = getAttrValue(object, "__getitem__");

    Value result = call(
        getItem,
        {
            Value(
                Value::BigInt(index)
            )
        },
        {},
        nullptr
    );

    --index;

    return result;
}

bool ReversedSequenceIterator::hasNext() const {
    return index >= 0;
}

QString ReversedSequenceIterator::getTypeName() const {
    return "reversed_sequence_iterator";
}
