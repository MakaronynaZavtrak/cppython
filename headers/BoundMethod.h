//
// Created by semyo on 01.05.2026.
//

#ifndef CPPYTHON_BOUNDMETHOD_H
#define CPPYTHON_BOUNDMETHOD_H
#include "ReprMixin.h"
#include "Value.h"

class FunctionValue;
class InstanceValue;
class ClassValue;

class BoundMethod : public ReprMixin {
public:
    Value callable;
    std::shared_ptr<InstanceValue> instance;
    std::shared_ptr<ClassValue> ownerClass;

    BoundMethod(Value callable,
                std::shared_ptr<InstanceValue> inst,
                std::shared_ptr<ClassValue> ownerClass)
        : callable(std::move(callable)), instance(std::move(inst)), ownerClass(std::move(ownerClass)) {}



    [[nodiscard]] QString toString() const override;

    static QString getCallableName(const Value& v);

    static QString getCallableOwner(const Value& v);
};
#endif //CPPYTHON_BOUNDMETHOD_H