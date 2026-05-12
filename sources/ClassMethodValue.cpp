#include "ClassMethodValue.h"

#include "BoundMethod.h"
#include "FunctionValue.h"
//
// Created by semyo on 11.05.2026.
//
Value ClassMethodValue::get(const Value&,
                            const std::shared_ptr<ClassValue>& owner) const {

    return Value(std::make_shared<BoundMethod>(Value(func), Value(owner), owner));
}

QString ClassMethodValue::toString() const {
    return func->toString();
}
