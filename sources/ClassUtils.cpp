//
// Created by semyo on 03.05.2026.
//
#include "ClassUtils.h"
#include "ClassValue.h"

Value findAttr(const Value::ClassPtr& cls, const QString& attr) {
    // 1. сначала в самом классе
    if (cls->attributes.contains(attr)) {
        return cls->attributes[attr];
    }

    // 2. потом в базовых классах
    for (const auto& base : cls->bases) {
        try {
            return findAttr(base, attr);
        } catch (...) {
            // просто идём дальше
        }
    }

    throw std::runtime_error("Attribute not found: " + attr.toStdString());
}

bool hasAttr(const Value::ClassPtr& cls, const QString& attr) {
    // 1. проверяем текущий класс
    if (cls->attributes.contains(attr)) {
        return true;
    }

    // 2. проверяем базовые классы
    for (const auto& base : cls->bases) {
        if (hasAttr(base, attr)) {
            return true;
        }
    }

    return false;
}