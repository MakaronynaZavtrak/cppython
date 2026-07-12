//
// Created by semyo on 07.07.2026.
//

#ifndef CPPYTHON_GENERATORVALUE_H
#define CPPYTHON_GENERATORVALUE_H
#include <memory>

#include "IteratorValue.h"
#include "Value.h"

class GeneratorValue : public std::enable_shared_from_this<GeneratorValue>, public IteratorValue {
public:
    std::shared_ptr<FunctionValue> func;
    std::shared_ptr<Environment> env; // окружение вызова, с уже забинженными аргументами
    std::vector<size_t> resumePath;
    std::vector<Value> resumeIterators;
    std::vector<Value> resumeGuardInstances;
    std::vector<std::exception_ptr> resumePendingExceptions;
    bool started = false;
    bool finished = false;

    Value next() override;

    [[nodiscard]] bool hasNext() const override;

    [[nodiscard]] QString getTypeName() const override;

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] QString repr() const override;

    Value send(const Value& value);

    Value throwInto(const Value& excValue);

    Value close();
};
#endif //CPPYTHON_GENERATORVALUE_H