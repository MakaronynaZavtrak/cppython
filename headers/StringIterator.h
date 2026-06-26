//
// Created by semyo on 24.05.2026.
//

#ifndef CPPYTHON_STRINGITERATOR_H
#define CPPYTHON_STRINGITERATOR_H
#include "IteratorValue.h"

class StringIterator final : public IteratorValue {

    QString value;
    int index = 0;

public:

    explicit StringIterator(QString value)
        : value(std::move(value)) {}

    Value next() override;

    [[nodiscard]] bool hasNext() const override;

    [[nodiscard]] QString getTypeName() const override;
};
#endif //CPPYTHON_STRINGITERATOR_H