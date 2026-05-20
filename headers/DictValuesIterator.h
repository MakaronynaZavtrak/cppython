//
// Created by semyo on 19.05.2026.
//

#ifndef CPPYTHON_DICTVALUEITERATOR_H
#define CPPYTHON_DICTVALUEITERATOR_H
#include <memory>
class DictValue;
class DictItemsView;

class DictValuesIterator {
public:

    std::shared_ptr<DictValue> dict;
    std::size_t index = 0;

    explicit DictValuesIterator(std::shared_ptr<DictValue> dict)
    : dict(std::move(dict)) {}

    [[nodiscard]] Value next() {

        if (index >= static_cast<size_t>(dict->getOrder().size())) {
            throw StopIterationException();
        }

        const QString key = dict->getOrder()[index++];

        return dict->getElements()[key];
    }
};
#endif //CPPYTHON_DICTVALUEITERATOR_H