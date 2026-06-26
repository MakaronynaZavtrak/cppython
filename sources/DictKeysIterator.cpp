//
// Created by semyo on 23.05.2026.
//

#include "DictKeysIterator.h"
#include "DictValue.h"
#include "StopIterationException.h"

Value DictKeysIterator::next() {

    if (!hasNext()) {
        throw StopIterationException();
    }

    return dict->getOrder()[index++];
}

bool DictKeysIterator::hasNext() const {
    return index < static_cast<size_t>(dict->getOrder().size());
}

QString DictKeysIterator::getTypeName() const {
    return "dict_keyiterator";
}
