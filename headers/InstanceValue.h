//
// Created by semyo on 01.05.2026.
//

#ifndef CPPYTHON_INSTANCEVALUE_H
#define CPPYTHON_INSTANCEVALUE_H
#include "ClassValue.h"

class InstanceValue {
public:
    std::shared_ptr<ClassValue> klass;
    QMap<QString, Value> fields;

    explicit InstanceValue(std::shared_ptr<ClassValue> cls)
    : klass(std::move(cls)) {}

    QString toString() const {
        QString addr = QString("0x%1")
            .arg(reinterpret_cast<quintptr>(this), 0, 16);

        return QString("<%1.%2 object at %3>")
                .arg("__main__", klass->name, addr);
    }
};
#endif //CPPYTHON_INSTANCEVALUE_H