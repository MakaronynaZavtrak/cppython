//
// Created by semyo on 22.09.2026.
//

#ifndef CPPYTHON_ZIPITERATOR_H
#define CPPYTHON_ZIPITERATOR_H
#include <vector>

#include "IteratorValue.h"
#include "Value.h"

class ZipIterator : public IteratorValue {
public:
    std::vector<Value::IteratorPtr> sources;
    bool strict;

    ZipIterator(std::vector<Value::IteratorPtr> sources, bool strict)
        : sources(std::move(sources)),
          strict(strict) {}

    Value next() override;

    [[nodiscard]] bool hasNext() const override;

    [[nodiscard]] QString getTypeName() const override;

private:
    mutable bool finished = false;
};
#endif //CPPYTHON_ZIPITERATOR_H