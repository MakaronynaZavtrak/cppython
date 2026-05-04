//
// Created by semyo on 01.05.2026.
//

#ifndef CPPYTHON_BOUNDMETHOD_H
#define CPPYTHON_BOUNDMETHOD_H
#include "FunctionValue.h"
#include "InstanceValue.h"

class BoundMethod {
public:
    std::shared_ptr<FunctionValue> function;
    std::shared_ptr<InstanceValue> instance;
    std::shared_ptr<ClassValue> ownerClass;

    BoundMethod(std::shared_ptr<FunctionValue> func,
                std::shared_ptr<InstanceValue> inst,
                std::shared_ptr<ClassValue> ownerClass)
        : function(std::move(func)), instance(std::move(inst)), ownerClass(std::move(ownerClass)) {}

    [[nodiscard]] QString toString() const;
};
#endif //CPPYTHON_BOUNDMETHOD_H