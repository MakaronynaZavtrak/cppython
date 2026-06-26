//
// Created by semyo on 24.05.2026.
//
#include "StringIterator.h"
#include "StopIterationException.h"
#include "Value.h"

bool StringIterator::hasNext() const {
    return index < value.size();
}

Value StringIterator::next() {

    if (!hasNext()) {
        throw StopIterationException();
    }

    return Value(QString(value[index++]));
}

QString StringIterator::getTypeName() const {
    return "str_ascii_iterator";
}
