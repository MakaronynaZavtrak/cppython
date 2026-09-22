//
// Created by semyo on 23.09.2026.
//

#include "EnumerateIterator.h"

#include "TupleValue.h"
#include "../exception/StopIterationException.h"

Value EnumerateIterator::next() {

    if (finished) {
        throw StopIterationException();
    }

    Value item;

    try {
        item = source->next();
    } catch (const StopIterationException &) {
        finished = true;
        throw;
    }

    // (текущий индекс, элемент)
    std::vector<Value> pair = { Value(index), item };

    index += 1;

    return Value(std::make_shared<TupleValue>(pair));
}

bool EnumerateIterator::hasNext() const {
    return !finished;
}

QString EnumerateIterator::getTypeName() const {
    return "enumerate";
}