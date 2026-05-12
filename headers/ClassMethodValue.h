//
// Created by semyo on 11.05.2026.
//

#ifndef CPPYTHON_CLASSMETHODVALUE_H
#define CPPYTHON_CLASSMETHODVALUE_H
#include <memory>

#include "ReprMixin.h"
class ClassValue;
class InstanceValue;
class FunctionValue;
class Value;

class ClassMethodValue : public ReprMixin {
public:
    std::shared_ptr<FunctionValue> func;

    explicit ClassMethodValue(std::shared_ptr<FunctionValue> f)
        : func(std::move(f)) {}

    [[nodiscard]] Value get(const Value& instance,
                            const std::shared_ptr<ClassValue>& owner) const;

    [[nodiscard]] QString toString() const override;
};
#endif //CPPYTHON_CLASSMETHODVALUE_H