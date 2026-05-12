#include "FunctionValue.h"

#include "BoundMethod.h"
#include "Value.h"
//
// Created by semyo on 03.05.2026.
//
Value FunctionValue::get(const Value& instance, const std::shared_ptr<ClassValue>& owner) {

    // доступ через класс
    if (instance.isNone()) {
        return Value(shared_from_this());
    }

    // доступ через instance
    return Value(std::make_shared<BoundMethod>(
        Value(shared_from_this()),
        instance,
        owner
    ));
}

QString FunctionValue::toString() const {
    QString addr = QString("0x%1")
       .arg(reinterpret_cast<quintptr>(this), 0, 16);

    if (ownerClass) {
        QString ownerName = ownerClass->name;

        if (!ownerName.isEmpty()) {
            return QString("<function %1.%2 at %3>")
                .arg(ownerName, name, addr);
        }
    }

    return QString("<function %1 at %2>")
        .arg(name, addr);
}
