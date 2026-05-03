#include "FunctionValue.h"

#include "BoundMethod.h"
#include "Value.h"
//
// Created by semyo on 03.05.2026.
//
Value FunctionValue::get(std::shared_ptr<InstanceValue> instance, std::shared_ptr<ClassValue> owner) {
    if (instance) {
        return Value(std::make_shared<BoundMethod>(shared_from_this(), instance));
    }
    return Value(shared_from_this());
}

QString FunctionValue::toString() const {
    QString addr = QString("0x%1")
       .arg(reinterpret_cast<quintptr>(this), 0, 16);

    if (!ownerClassName.isEmpty()) {
        return QString("<function %1.%2 at %3>")
            .arg(ownerClassName, name, addr);
    }

    return QString("<function %1 at %2>")
        .arg(name, addr);
}
