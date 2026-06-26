//
// Created by semyo on 15.06.2026.
//
#include "ByteArrayIterator.h"

#include "StopIterationException.h"
#include "Value.h"

Value ByteArrayIterator::next() {

    if (!hasNext()) {
        throw StopIterationException();
    }

    const unsigned char value =
        static_cast<unsigned char>(
            snapshot[index++]
        );

    return Value(
        Value::BigInt(
            std::to_string(value)
        )
    );
}

bool ByteArrayIterator::hasNext() const {

    return index < snapshot.size();
}

QString ByteArrayIterator::getTypeName() const {
    return "bytearray_iterator";
}