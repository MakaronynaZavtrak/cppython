//
// Created by semyo on 23.05.2026.
//

#ifndef CPPYTHON_SETITERATOR_H
#define CPPYTHON_SETITERATOR_H
#include <memory>

#include "IteratorValue.h"
#include "SetValue.h"

class SetIterator : public IteratorValue {
public:
    std::shared_ptr<SetValue> set;
    int index = 0;

    explicit SetIterator(std::shared_ptr<SetValue> set)
        : set(std::move(set)) {}

    Value next() override;

    [[nodiscard]] bool hasNext() const override;

    [[nodiscard]] QString getTypeName() const override;
};
#endif //CPPYTHON_SETITERATOR_H