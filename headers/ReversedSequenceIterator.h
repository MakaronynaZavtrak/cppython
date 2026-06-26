//
// Created by semyo on 22.06.2026.
//

#ifndef CPPYTHON_REVERSEDSEQUENCEITERATOR_H
#define CPPYTHON_REVERSEDSEQUENCEITERATOR_H
#include "IteratorValue.h"
#include "Value.h"

class ReversedSequenceIterator : public IteratorValue {

    Value object;
    std::ptrdiff_t index;

public:

    ReversedSequenceIterator(Value object, qsizetype length);

    Value next() override;

    [[nodiscard]] bool hasNext() const override;

    [[nodiscard]] QString getTypeName() const override;
};
#endif //CPPYTHON_REVERSEDSEQUENCEITERATOR_H