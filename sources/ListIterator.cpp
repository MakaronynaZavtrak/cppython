//
// Created by semyo on 23.05.2026.
//
#include "ListIterator.h"
#include "ListValue.h"
#include "StopIterationException.h"

Value ListIterator::next() {

    if (!hasNext()) {
        throw StopIterationException();
    }

    return list->elements[index++];
}

bool ListIterator::hasNext() const {
    return index < list->elements.size();
}

QString ListIterator::getTypeName() const {
    return "list_iterator";
}
