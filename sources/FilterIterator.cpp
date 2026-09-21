//
// Created by semyo on 22.09.2026.
//

#include "FilterIterator.h"

#include "CallRuntime.h"
#include "../exception/StopIterationException.h"

void FilterIterator::advance() const {

    // элемент уже подготовлен или источник исчерпан — делать нечего
    if (hasBuffered || finished) {
        return;
    }

    while (true) {

        Value item;

        try {
            item = source->next();
        } catch (const StopIterationException &) {
            finished = true;
            return;
        }

        // None-предикат означает отбор по истинности самого элемента
        const bool keep = predicate.isNone()
            ? item.toBool()
            : call(predicate, {item}, {}, env).toBool();

        if (keep) {
            buffered = item;
            hasBuffered = true;
            return;
        }
        // иначе продолжаем искать подходящий элемент
    }
}

Value FilterIterator::next() {

    advance();

    if (!hasBuffered) {
        throw StopIterationException();
    }

    Value result = buffered;

    buffered = Value();
    hasBuffered = false;

    return result;
}

bool FilterIterator::hasNext() const {
    advance();
    return hasBuffered;
}

QString FilterIterator::getTypeName() const {
    return "filter";
}