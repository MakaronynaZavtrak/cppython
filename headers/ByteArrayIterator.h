//
// Created by semyo on 15.06.2026.
//

#ifndef CPPYTHON_BYTEARRAYITERATOR_H
#define CPPYTHON_BYTEARRAYITERATOR_H
#include "IteratorValue.h"

class ByteArrayValue;

class ByteArrayIterator : public IteratorValue {

public:

    QByteArray snapshot;

    std::size_t index = 0;

    explicit ByteArrayIterator(const QByteArray& bytes)
    : snapshot(bytes) {}

    Value next() override;

    bool hasNext() const override;

    QString getTypeName() const override;
};
#endif //CPPYTHON_BYTEARRAYITERATOR_H