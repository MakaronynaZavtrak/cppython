//
// Created by semyo on 23.09.2026.
//

#ifndef CPPYTHON_ENUMERATEITERATOR_H
#define CPPYTHON_ENUMERATEITERATOR_H
#include "IteratorValue.h"
#include "Value.h"

class EnumerateIterator : public IteratorValue {
public:
    Value::IteratorPtr source;
    Value::BigInt index;

    EnumerateIterator(Value::IteratorPtr source, Value::BigInt start)
        : source(std::move(source)),
          index(std::move(start)) {}

    Value next() override;

    [[nodiscard]] bool hasNext() const override;

    [[nodiscard]] QString getTypeName() const override;

private:
    mutable bool finished = false;
};
#endif //CPPYTHON_ENUMERATEITERATOR_H