#include "InstanceValue.h"
//
// Created by semyo on 05.05.2026.
//
QString InstanceValue::toString() const {
    QString addr = QString("0x%1")
        .arg(reinterpret_cast<quintptr>(this), 0, 16);

    return QString("<%1.%2 object at %3>")
            .arg("__main__", klass->name, addr);
}
