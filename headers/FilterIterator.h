//
// Created by semyo on 22.09.2026.
//

#ifndef CPPYTHON_FILTERITERATOR_H
#define CPPYTHON_FILTERITERATOR_H
#include <memory>

#include "IteratorValue.h"
#include "Value.h"

class Environment;

class FilterIterator : public IteratorValue {
public:
    Value predicate;                 // если None — отбор по истинности элемента
    Value::IteratorPtr source;
    std::shared_ptr<Environment> env;

    FilterIterator(
        Value predicate,
        Value::IteratorPtr source,
        std::shared_ptr<Environment> env)
            : predicate(std::move(predicate)),
              source(std::move(source)),
              env(std::move(env)) {}

    Value next() override;

    [[nodiscard]] bool hasNext() const override;

    [[nodiscard]] QString getTypeName() const override;

private:
    // подтягивает следующий подходящий элемент в буфер (предпросмотр),
    // чтобы hasNext() мог честно ответить даже при отсеянном хвосте
    void advance() const;

    mutable Value buffered;
    mutable bool hasBuffered = false;
    mutable bool finished = false;
};
#endif //CPPYTHON_FILTERITERATOR_H