//
// Created by semyo on 21.09.2026.
//

#ifndef CPPYTHON_MAPITERATOR_H
#define CPPYTHON_MAPITERATOR_H
#include <memory>
#include <vector>

#include "IteratorValue.h"
#include "Value.h"

class Environment;

class MapIterator : public IteratorValue {
public:
    Value func;
    std::vector<Value::IteratorPtr> sources;
    std::shared_ptr<Environment> env;

    MapIterator(
        Value func,
        std::vector<Value::IteratorPtr> sources,
        std::shared_ptr<Environment> env)
            : func(std::move(func)),
              sources(std::move(sources)),
              env(std::move(env)) {}

    Value next() override;

    [[nodiscard]] bool hasNext() const override;

    [[nodiscard]] QString getTypeName() const override;

private:
    mutable bool finished = false;
};
#endif //CPPYTHON_MAPITERATOR_H