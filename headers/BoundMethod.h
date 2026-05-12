//
// Created by semyo on 01.05.2026.
//

#ifndef CPPYTHON_BOUNDMETHOD_H
#define CPPYTHON_BOUNDMETHOD_H
#include <utility>

#include "ReprMixin.h"
#include "Value.h"

class FunctionValue;
class InstanceValue;
class ClassValue;

class BoundMethod : public ReprMixin {
public:
    Value callable;
    Value self;
    std::shared_ptr<ClassValue> ownerClass;

    BoundMethod(Value callable,
                Value self,
                std::shared_ptr<ClassValue> ownerClass)
        : callable(std::move(callable)), self(std::move(self)), ownerClass(std::move(ownerClass)) {}



    [[nodiscard]] QString toString() const override;

    static QString getCallableName(const Value& v);

    static QString getCallableOwner(const Value& v);
};
#endif //CPPYTHON_BOUNDMETHOD_H