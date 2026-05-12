#include "StaticMethodValue.h"

#include "FunctionValue.h"
#include "Value.h"
//
// Created by semyo on 11.05.2026.
//
Value StaticMethodValue::get(const Value&,
                            const std::shared_ptr<ClassValue>&) const {

    return Value(func);
}

QString StaticMethodValue::toString() const {
    return func->toString();
}
