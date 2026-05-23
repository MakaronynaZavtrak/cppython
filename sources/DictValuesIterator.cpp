//
// Created by semyo on 23.05.2026.
//

#include "DictValuesIterator.h"

#include "DictValue.h"
#include "StopIterationException.h"

Value DictValuesIterator::next() {

    if (!hasNext()) {
        throw StopIterationException();
    }

    return dict->getElements()[dict->getOrder()[index++]];
}

bool DictValuesIterator::hasNext() const {
    return index < static_cast<size_t>(dict->getOrder().size());
}

QString DictValuesIterator::getTypeName() const {
    return "dict_valueiterator";
}
