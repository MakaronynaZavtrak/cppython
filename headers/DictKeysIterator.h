//
// Created by semyo on 19.05.2026.
//

#ifndef CPPYTHON_DICTKEYSITERATOR_H
#define CPPYTHON_DICTKEYSITERATOR_H
#include <memory>

#include "IteratorValue.h"

class DictValue;
class DictItemsView;

class DictKeysIterator : public IteratorValue {
public:

    std::shared_ptr<DictValue> dict;
    std::size_t index = 0;

    explicit DictKeysIterator(std::shared_ptr<DictValue> dict)
    : dict(std::move(dict)) {}

    Value next() override;

    [[nodiscard]] bool hasNext() const override;

    [[nodiscard]] QString getTypeName() const override;
};
#endif //CPPYTHON_DICTKEYSITERATOR_H