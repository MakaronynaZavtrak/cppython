#include "BoundMethod.h"
//
// Created by semyo on 03.05.2026.
//
QString BoundMethod::toString() const {
    QString funcName = function->name;
    QString className = function->ownerClassName;

    return QString("<bound method %1.%2 of %3>")
        .arg(className, funcName, instance->toString());
}