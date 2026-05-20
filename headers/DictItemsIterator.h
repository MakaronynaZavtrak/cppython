//
// Created by semyo on 19.05.2026.
//

#ifndef CPPYTHON_DICTITEMSITERATOR_H
#define CPPYTHON_DICTITEMSITERATOR_H
#include <memory>

#include "DictValue.h"
#include "StopIterationException.h"
#include "TupleValue.h"
class DictValue;
class DictItemsView;

class DictItemsIterator {
public:

    std::shared_ptr<DictValue> dict;
    std::size_t index = 0;

    explicit DictItemsIterator(std::shared_ptr<DictValue> dict)
    : dict(std::move(dict)) {}

    [[nodiscard]] Value next() {

        if (index >= static_cast<size_t>(dict->getOrder().size())) {
            throw StopIterationException();
        }

        const QString key = dict->getOrder()[index++];

        std::vector tupleItems = {
            Value(key),
            dict->getElements()[key]
        };

        return Value(std::make_shared<TupleValue>(tupleItems));
    }
};
#endif //CPPYTHON_DICTITEMSITERATOR_H