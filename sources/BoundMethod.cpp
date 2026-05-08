#include "BoundMethod.h"

#include "CallRuntime.h"
#include "FunctionValue.h"
//
// Created by semyo on 03.05.2026.
//
QString BoundMethod::toString() const {
    QString className = ownerClass ? ownerClass->name : getCallableOwner(callable);
    QString funcName = getCallableName(callable);
    QString instanceStr = instance ? instance->toString() : "<unknown instance>";

    return QString("<bound method %1.%2 of %3>")
        .arg(className, funcName, instanceStr);
}

QString BoundMethod::getCallableName(const Value &v) {
    if (std::holds_alternative<Value::FunctionPtr>(v.data)) {
        return std::get<Value::FunctionPtr>(v.data)->name;
    }

    if (std::holds_alternative<Value::BuiltinFunctionPtr>(v.data)) {
        return std::get<Value::BuiltinFunctionPtr>(v.data)->name;
    }

    return "<unknown>";
}

QString BoundMethod::getCallableOwner(const Value &v) {
    if (std::holds_alternative<Value::FunctionPtr>(v.data)) {

        const auto fn = std::get<Value::FunctionPtr>(v.data);
        if (fn->ownerClass) {
            return fn->ownerClass->name;
        }
    }
    return "";
}
