//
// Created by semyo on 03.05.2026.
//
#include "ClassUtils.h"
#include "ClassValue.h"
#include "DescriptorUtils.h"
#include "InstanceValue.h"

bool hasAttr(const Value::ClassPtr& cls, const QString& attr) {
    try {
        findAttrInHierarchy(cls, attr);
        return true;
    } catch (...) {
        return false;
    }
}

Value getAttrValue(const Value& obj, const QString& attr) {
    // instance
    if (std::holds_alternative<Value::InstancePtr>(obj.data)) {
        auto instance = std::get<Value::InstancePtr>(obj.data);
        auto cls = instance->klass;

        // 1. поля объекта
        if (instance->fields.contains(attr)) {
            return instance->fields[attr];
        }

        // 2. класс + наследование
        if (hasAttr(cls, attr)) {
            Value val = findAttrInHierarchy(cls, attr);
            return applyDescriptor(val, instance, cls);
        }
    }

    // super
    if (std::holds_alternative<Value::SuperPtr>(obj.data)) {
        auto super = std::get<Value::SuperPtr>(obj.data);
        return getAttrFromSuper(super, attr);
    }

    // class
    if (std::holds_alternative<Value::ClassPtr>(obj.data)) {
        auto cls = std::get<Value::ClassPtr>(obj.data);
        Value val = findAttrInHierarchy(cls, attr);
        return applyDescriptor(val, nullptr, cls);
    }

    throw std::runtime_error("AttributeError: object has no attribute '" +
                            attr.toStdString() + "'");
}

Value getAttrFromSuper(const Value::SuperPtr& super, const QString& attr) {
    for (const auto& base : super->currentClass->bases) {
        try {
            Value val = findAttrInHierarchy(base, attr);
            return applyDescriptor(val, super->instance, base);
        } catch (...) {}
    }

    throw std::runtime_error("AttributeError: object has no attribute '" +
                            attr.toStdString() + "'");
}

Value findAttrInHierarchy(const Value::ClassPtr& cls, const QString& attr) {
    if (cls->attributes.contains(attr)) {
        return cls->attributes[attr];
    }

    for (const auto& base : cls->bases) {
        try {
            return findAttrInHierarchy(base, attr);
        } catch (...) {}
    }

    throw std::runtime_error("Attribute not found: " + attr.toStdString());
}

void setAttrValue(const Value& obj, const QString& attr, const Value& value) {
    if (std::holds_alternative<Value::InstancePtr>(obj.data)) {
        const auto instance = std::get<Value::InstancePtr>(obj.data);
        instance->fields[attr] = value;
        return;
    }

    if (std::holds_alternative<Value::ClassPtr>(obj.data)) {
        const auto cls = std::get<Value::ClassPtr>(obj.data);
        cls->attributes[attr] = value;
        return;
    }

    throw std::runtime_error("setattr: object has no attributes");
}
