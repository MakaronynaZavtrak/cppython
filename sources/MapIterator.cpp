//
// Created by semyo on 21.09.2026.
//

#include "MapIterator.h"

#include "CallRuntime.h"
#include "../exception/StopIterationException.h"

Value MapIterator::next() {

    if (finished) {
        throw StopIterationException();
    }

    // из каждого источника берём по одному элементу
    std::vector<Value> args;
    args.reserve(sources.size());

    for (const auto &source: sources) {

        try {
            args.push_back(source->next());
        } catch (const StopIterationException &) {
            // любой исчерпанный источник завершает map (останов по кратчайшему)
            finished = true;
            throw;
        }
    }

    // применяем функцию к собранному кортежу аргументов
    return call(func, args, {}, env);
}

bool MapIterator::hasNext() const {
    return !finished;
}

QString MapIterator::getTypeName() const {
    return "map";
}