//
// Created by semyo on 11.05.2026.
//

#ifndef CPPYTHON_STATICMETHODVALUE_H
#define CPPYTHON_STATICMETHODVALUE_H
#include <memory>

#include "ReprMixin.h"
class ClassValue;
class InstanceValue;
class FunctionValue;
class Value;

class StaticMethodValue : public ReprMixin {
public:
    std::shared_ptr<FunctionValue> func;

    explicit StaticMethodValue(std::shared_ptr<FunctionValue> f)
        : func(std::move(f)) {}

    [[nodiscard]] Value get(const Value&,
                            const std::shared_ptr<ClassValue>&) const;

    [[nodiscard]] QString toString() const override;
};
#endif //CPPYTHON_STATICMETHODVALUE_H