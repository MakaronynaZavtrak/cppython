//
// Created by semyo on 18.05.2026.
//

#ifndef CPPYTHON_TUPLEITERATORVALUE_H
#define CPPYTHON_TUPLEITERATORVALUE_H
#include <memory>
class TupleValue;

class TupleIteratorValue {
public:

    std::shared_ptr<TupleValue> tuple;
    std::size_t index = 0;

    explicit TupleIteratorValue(std::shared_ptr<TupleValue> tuple)
    : tuple(std::move(tuple)) {}
};
#endif //CPPYTHON_TUPLEITERATORVALUE_H