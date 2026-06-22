//
// Created by semyo on 22.06.2026.
//

#ifndef CPPYTHON_REVERSEDDICTITERATOR_H
#define CPPYTHON_REVERSEDDICTITERATOR_H
#include "IteratorValue.h"
#include "DictValue.h"

class ReversedDictIterator : public IteratorValue {

    std::shared_ptr<const DictValue> dict;
    std::ptrdiff_t index;

public:

    explicit ReversedDictIterator(
        std::shared_ptr<const DictValue> dict
    );

    Value next() override;

    [[nodiscard]]
    bool hasNext() const override;

    [[nodiscard]]
    QString getTypeName() const override;
};
#endif //CPPYTHON_REVERSEDDICTITERATOR_H