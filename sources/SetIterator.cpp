//
// Created by semyo on 23.05.2026.
//
#include "SetIterator.h"
#include "StopIterationException.h"
#include "Value.h"

Value SetIterator::next() {

    if (!hasNext()) {
        throw StopIterationException();
    }

    return set->order[index++];
}

bool SetIterator::hasNext() const {
    return index < set->order.size();
}

QString SetIterator::getTypeName() const {
    return "set_iterator";
}
