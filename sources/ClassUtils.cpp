//
// Created by semyo on 03.05.2026.
//
#include "ClassUtils.h"
#include "ClassValue.h"
#include "InstanceValue.h"
#include "SuperValue.h"

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

        //  1. data descriptor (hasSet)
        try {
            Value val = findAttrInHierarchy(cls, attr);

            if (val.hasGet() && val.hasSet()) {
                return val.callGet(instance, cls);
            }
        } catch (const std::runtime_error& e) {

            const std::string msg = e.what();

            if (msg.find("Attribute not found") == std::string::npos) {
                throw;
            }
        }

        // 2. instance fields
        if (instance->fields.contains(attr)) {
            return instance->fields[attr];
        }

        // 3. non-data descriptor | class attribute
        try {
            Value val = findAttrInHierarchy(cls, attr);

            if (val.hasGet()) {
                return val.callGet(instance, cls);
            }

            return val;
        } catch (const std::runtime_error& e) {

            const std::string msg = e.what();

            if (msg.find("Attribute not found") == std::string::npos) {
                throw;
            }
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

        if (val.hasGet()) {
            return val.callGet(nullptr, cls);
        }

        return val;
    }

    throw std::runtime_error("AttributeError: object has no attribute '" +
                            attr.toStdString() + "'");
}

Value getAttrFromSuper(const Value::SuperPtr& super, const QString& attr) {
    for (const auto& base : super->currentClass->bases) {
        try {
            Value val = findAttrInHierarchy(base, attr);

            if (val.hasGet()) {
                return val.callGet(super->instance, base);
            }

            return val;

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
    // instance
    if (std::holds_alternative<Value::InstancePtr>(obj.data)) {
        const auto instance = std::get<Value::InstancePtr>(obj.data);
        const auto cls = instance->klass;

        // 1. проверяем descriptor в классе
        try {
            const Value descr = findAttrInHierarchy(cls, attr);

            if (descr.hasSet()) {
                descr.callSet(instance, cls, value);
                return;
            }
        } catch (const std::runtime_error& e) {
            const std::string msg = e.what();
            if (msg.find("Attribute not found") == std::string::npos) {
                throw;
            }
        }

        // 2. обычная запись в поля
        instance->fields[attr] = value;
        return;
    }

    // class
    if (std::holds_alternative<Value::ClassPtr>(obj.data)) {
        const auto cls = std::get<Value::ClassPtr>(obj.data);
        cls->attributes[attr] = value;
        return;
    }

    throw std::runtime_error("setattr: object has no attributes");
}
