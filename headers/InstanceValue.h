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
};
#endif //CPPYTHON_INSTANCEVALUE_H