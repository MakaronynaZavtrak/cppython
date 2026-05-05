//
// Created by semyo on 05.05.2026.
//

#ifndef CPPYTHON_PROPERTYVALUE_H
#define CPPYTHON_PROPERTYVALUE_H
#include <memory>

class FunctionValue;
class InstanceValue;
class ClassValue;

class Value;

class PropertyValue {
public:
    std::shared_ptr<FunctionValue> fget;
    std::shared_ptr<FunctionValue> fset;
    std::shared_ptr<FunctionValue> fdel;

    explicit PropertyValue(std::shared_ptr<FunctionValue> get = nullptr,
                           std::shared_ptr<FunctionValue> set = nullptr,
                           std::shared_ptr<FunctionValue> del = nullptr)
        : fget(std::move(get)), fset(std::move(set)), fdel(std::move(del)) {}

    Value get(const std::shared_ptr<InstanceValue>&, const std::shared_ptr<ClassValue>&);
};
#endif //CPPYTHON_PROPERTYVALUE_H