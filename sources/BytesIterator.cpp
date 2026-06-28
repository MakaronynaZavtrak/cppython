//
// Created by semyo on 04.06.2026.
//
#include "BytesIterator.h"

#include "BytesValue.h"
#include "../exception/StopIterationException.h"
#include "Value.h"

Value BytesIterator::next() {

    if (!hasNext()) {
        throw StopIterationException();
    }

    const auto value =
        static_cast<unsigned char>(
            bytes->bytes()[index++]
        );

    return Value(Value::BigInt(std::to_string(value)));
}

bool BytesIterator::hasNext() const {
    return index < bytes->bytes().size();
}

QString BytesIterator::getTypeName() const {
    return "bytes_iterator";
}