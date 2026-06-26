//
// Created by semyo on 19.05.2026.
//

#ifndef CPPYTHON_DICTITEMSITERATOR_H
#define CPPYTHON_DICTITEMSITERATOR_H
#include <memory>

#include "IteratorValue.h"
#include "TupleValue.h"

class DictValue;
class DictItemsView;

class DictItemsIterator : public IteratorValue {
public:

    std::shared_ptr<DictValue> dict;
    std::size_t index = 0;

    explicit DictItemsIterator(std::shared_ptr<DictValue> dict)
    : dict(std::move(dict)) {}

    Value next() override;

    [[nodiscard]] bool hasNext() const override;

    [[nodiscard]] QString getTypeName() const override;
};
#endif //CPPYTHON_DICTITEMSITERATOR_H