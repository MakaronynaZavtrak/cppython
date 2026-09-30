//
// Created by semyo on 22.09.2026.
//

#include "ZipIterator.h"

#include "TupleValue.h"
#include "../exception/StopIterationException.h"
#include "../exception/ValueErrorException.h"

Value ZipIterator::next() {

    if (finished) {
        throw StopIterationException();
    }

    // zip() без источников — сразу пустой итератор
    if (sources.empty()) {
        finished = true;
        throw StopIterationException();
    }

    std::vector<Value> tuple;
    tuple.reserve(sources.size());

    for (std::size_t i = 0; i < sources.size(); ++i) {

        Value item;

        try {
            item = sources[i]->next();
        } catch (const StopIterationException &) {

            finished = true;

            if (!strict) {
                // обычный режим — останов по кратчайшему источнику
                throw;
            }

            // strict: длины источников должны совпадать
            if (i > 0) {
                // предыдущие источники ещё выдавали элемент, а этот уже нет
                throw ValueErrorException(
                    "zip() argument " + QString::number(i + 1) +
                    " is shorter than argument 1"
                );
            }

            // первый источник исчерпан — проверяем, что и остальные исчерпаны
            for (std::size_t j = 1; j < sources.size(); ++j) {

                try {
                    sources[j]->next();
                } catch (const StopIterationException &) {
                    continue;
                }

                // какой-то последующий источник ещё выдал элемент — он длиннее
                throw ValueErrorException(
                    "zip() argument " + QString::number(j + 1) +
                    " is longer than argument 1"
                );
            }

            // все источники исчерпаны одновременно — обычное завершение
            throw;
        }

        tuple.push_back(item);
    }

    return Value(std::make_shared<TupleValue>(tuple));
}

bool ZipIterator::hasNext() const {
    return !finished;
}

QString ZipIterator::getTypeName() const {
    return "zip";
}