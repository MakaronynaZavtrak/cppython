//
// Created by semyo on 19.05.2026.
//

#ifndef CPPYTHON_DICTKEYSITERATOR_H
#define CPPYTHON_DICTKEYSITERATOR_H
#include <memory>

#include "DictValue.h"
#include "StopIterationException.h"
class DictValue;
class DictItemsView;

class DictKeysIterator {
public:

    std::shared_ptr<DictValue> dict;
    std::size_t index = 0;

    explicit DictKeysIterator(std::shared_ptr<DictValue> dict)
    : dict(std::move(dict)) {}

    [[nodiscard]] Value next() {

        if (index >= static_cast<size_t>(dict->getOrder().size())) {
            throw StopIterationException();
        }

        const Value key = dict->getOrder()[index++];

        return key;
    }
};
#endif //CPPYTHON_DICTKEYSITERATOR_H