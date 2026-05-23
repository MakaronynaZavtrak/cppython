//
// Created by semyo on 19.05.2026.
//

#ifndef CPPYTHON_DICTVALUEITERATOR_H
#define CPPYTHON_DICTVALUEITERATOR_H
#include <memory>

#include "IteratorValue.h"

class DictValue;
class DictItemsView;

class DictValuesIterator : public IteratorValue {
public:

    std::shared_ptr<DictValue> dict;
    std::size_t index = 0;

    explicit DictValuesIterator(std::shared_ptr<DictValue> dict)
    : dict(std::move(dict)) {}

    Value next() override;

    [[nodiscard]] bool hasNext() const override;

    [[nodiscard]] QString getTypeName() const override;
};
#endif //CPPYTHON_DICTVALUEITERATOR_H