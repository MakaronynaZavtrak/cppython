//
// Created by semyo on 18.05.2026.
//

#ifndef CPPYTHON_LISTITERATORVALUE_H
#define CPPYTHON_LISTITERATORVALUE_H
#include <memory>
class ListValue;

class ListIterator {
public:

    std::shared_ptr<ListValue> list;
    std::size_t index = 0;

    explicit ListIterator(std::shared_ptr<ListValue> list)
    : list(std::move(list)) {}
};
#endif //CPPYTHON_LISTITERATORVALUE_H