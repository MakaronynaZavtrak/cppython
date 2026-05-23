//
// Created by semyo on 23.05.2026.
//

#include "DictItemsIterator.h"
#include "DictValue.h"
#include "StopIterationException.h"

Value DictItemsIterator::next() {

    if (!hasNext()) {
        throw StopIterationException();
    }

    const Value key = dict->getOrder()[index++];

    std::vector tupleItems = {
        Value(key),
        dict->getElements()[key]
    };

    return Value(std::make_shared<TupleValue>(tupleItems));
}

bool DictItemsIterator::hasNext() const {
    return index < static_cast<size_t>(dict->getOrder().size());
}

QString DictItemsIterator::getTypeName() const {
    return "dict_itemiterator";
}
