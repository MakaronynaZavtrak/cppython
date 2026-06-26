//
// Created by semyo on 04.06.2026.
//

#ifndef CPPYTHON_BYTESITERATOR_H
#define CPPYTHON_BYTESITERATOR_H
#include "IteratorValue.h"

class BytesValue;

class BytesIterator : public IteratorValue {
public:

    std::shared_ptr<BytesValue> bytes;
    std::size_t index = 0;

    explicit BytesIterator(std::shared_ptr<BytesValue> bytes)
        : bytes(std::move(bytes)) {}

    Value next() override;

    [[nodiscard]] bool hasNext() const override;

    [[nodiscard]] QString getTypeName() const override;
};
#endif //CPPYTHON_BYTESITERATOR_H