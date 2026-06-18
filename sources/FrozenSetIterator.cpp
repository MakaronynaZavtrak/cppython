//
// Created by semyo on 18.06.2026.
//
#include "FrozenSetIterator.h"

#include "FrozenSetValue.h"
#include "StopIterationException.h"

Value FrozenSetIterator::next() {

    if (!hasNext()) {
        throw StopIterationException();
    }

    return frozenSet->getOrder()[index++];
}

bool FrozenSetIterator::hasNext() const {
    return index < frozenSet->getOrder().size();
}

QString FrozenSetIterator::getTypeName() const {
    return "frozenset_iterator";
}