//
// Created by semyo on 23.05.2026.
//
#include "TupleIterator.h"

#include "StopIterationException.h"
#include "TupleValue.h"

Value TupleIterator::next() {

    if (!hasNext()) {
        throw StopIterationException();
    }

    return tuple->items[index++];
}

bool TupleIterator::hasNext() const {
    return index < tuple->items.size();
}

QString TupleIterator::getTypeName() const {
    return "tuple_iterator";
}
