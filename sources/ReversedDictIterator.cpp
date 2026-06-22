//
// Created by semyo on 22.06.2026.
//
#include "ReversedDictIterator.h"

#include "StopIterationException.h"

ReversedDictIterator::ReversedDictIterator(std::shared_ptr<const DictValue> dict)
    : dict(std::move(dict)),
      index(this->dict->getOrder().size() - 1) {}

Value ReversedDictIterator::next() {

    if (!hasNext()) {
        throw StopIterationException();
    }

    return dict->getOrder()[index--];
}

bool ReversedDictIterator::hasNext() const {
    return index >= 0;
}

QString ReversedDictIterator::getTypeName() const {
    return "dict_reversekeyiterator";
}