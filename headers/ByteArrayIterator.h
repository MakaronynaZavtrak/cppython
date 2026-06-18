//
// Created by semyo on 15.06.2026.
//

#ifndef CPPYTHON_BYTEARRAYITERATOR_H
#define CPPYTHON_BYTEARRAYITERATOR_H
#include <utility>

#include "IteratorValue.h"

class ByteArrayValue;

class ByteArrayIterator : public IteratorValue {

public:

    QByteArray snapshot;

    std::size_t index = 0;

    explicit ByteArrayIterator(QByteArray  bytes)
    : snapshot(std::move(bytes)) {}

    Value next() override;

    [[nodiscard]] bool hasNext() const override;

    [[nodiscard]] QString getTypeName() const override;
};
#endif //CPPYTHON_BYTEARRAYITERATOR_H