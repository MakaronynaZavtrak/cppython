#include "BoundMethod.h"

#include "CallRuntime.h"
#include "FunctionValue.h"
//
// Created by semyo on 03.05.2026.
//
QString BoundMethod::toString() const {

    QString className = ownerClass ? ownerClass->name : getCallableOwner(callable);
    QString funcName = getCallableName(callable);
    QString instanceStr = !self.isNone() ? self.toString() : "<unknown instance>";

    return QString("<bound method %1.%2 of %3>")
        .arg(className, funcName, instanceStr);
}

QString BoundMethod::getCallableName(const Value &v) {

    if (v.isFunction()) {
        return v.asFunction()->name;
    }

    if (v.isBuiltinFunction()) {
        return v.asBuiltinFunction()->name;
    }

    return "<unknown>";
}

QString BoundMethod::getCallableOwner(const Value &v) {

    if (v.isFunction()) {

        if (const auto fn = v.asFunction();
            fn->ownerClass) {

            return fn->ownerClass->name;
        }
    }
    return "";
}
