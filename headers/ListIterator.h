//
// Created by semyo on 18.05.2026.
//

#ifndef CPPYTHON_LISTITERATORVALUE_H
#define CPPYTHON_LISTITERATORVALUE_H
#include <memory>

#include "IteratorValue.h"

class ListValue;

class ListIterator : public IteratorValue {
public:

    std::shared_ptr<ListValue> list;
    std::size_t index = 0;

    explicit ListIterator(std::shared_ptr<ListValue> list)
    : list(std::move(list)) {}

    Value next() override;

    [[nodiscard]] bool hasNext() const override;

    [[nodiscard]] QString getTypeName() const override;
};
#endif //CPPYTHON_LISTITERATORVALUE_H