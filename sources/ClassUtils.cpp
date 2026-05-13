//
// Created by semyo on 03.05.2026.
//
#include "ClassUtils.h"

#include "CallRuntime.h"
#include "ClassValue.h"
#include "DescriptorUtils.h"
#include "InstanceValue.h"
#include "ListValue.h"
#include "SuperValue.h"

bool hasAttr(const Value::ClassPtr& cls, const QString& attr) {
    try {
        findAttrInHierarchy(cls, attr);
        return true;
    } catch (...) {
        return false;
    }
}

Value genericGetAttr(const Value& obj, const QString& attr) {

    // instance
    if (std::holds_alternative<Value::InstancePtr>(obj.data)) {
        auto instance = std::get<Value::InstancePtr>(obj.data);
        auto cls = instance->klass;

        //  1. data descriptor (hasSet)
        try {
            Value val = findAttrInHierarchy(cls, attr);

            if (DescriptorUtils::hasGet(val) && DescriptorUtils::hasSet(val)) {
                return DescriptorUtils::callGet(val, Value(instance), cls);
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

            if (DescriptorUtils::hasGet(val)) {
                return DescriptorUtils::callGet(val, Value(instance), cls);
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

        if (DescriptorUtils::hasGet(val)) {
            return DescriptorUtils::callGet(val, Value(), cls);
        }

        return val;
    }

    if (std::holds_alternative<Value::ListPtr>(obj.data)) {

        auto list = std::get<Value::ListPtr>(obj.data);

        if (attr == "__getitem__") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "__getitem__",

                    [list](const std::vector<Value>& args,
                           const std::shared_ptr<Environment>&)
                           -> Value {

                        if (args.size() != 1) {
                            throw std::runtime_error(
                                "__getitem__ expects 1 arg"
                            );
                        }

                        return list->getItem(args[0]);
                    }
                )
            );
        }

        if (attr == "__setitem__") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "__setitem__",

                    [list](const std::vector<Value>& args,
                          const std::shared_ptr<Environment>&)
                          -> Value {

                        if (args.size() != 2) {
                            throw std::runtime_error(
                                "__setitem__ expects 2 args"
                            );
                        }

                        list->setItem(args[0], args[1]);

                        return {};
                    }
                )
            );
        }

        if (attr == "append") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "append",

                    [list](const std::vector<Value>& args,
                          const std::shared_ptr<Environment>&)
                          -> Value {

                        if (args.size() != 1) {
                            throw std::runtime_error(
                                "append expects 1 argument"
                            );
                        }

                        list->append(args[0]);

                        return {};
                    }
                )
            );
        }

        if (attr == "pop") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "pop",

                    [list](const std::vector<Value>& args,
                           const std::shared_ptr<Environment>&)
                           -> Value {

                        if (args.empty()) {
                            return list->pop();
                        }

                        if (args.size() == 1) {
                            return list->pop(args[0]);
                        }

                        throw std::runtime_error(
                            "pop expects at most 1 arg"
                        );
                    }
                )
            );
        }

        if (attr == "__len__") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "__len__",

                    [list](const std::vector<Value>& args,
                           const std::shared_ptr<Environment>&)
                           -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("__len__ expects 0 args");
                        }

                        return Value(Value::BigInt(list->len()));
                    }
                )
            );
        }

        if (attr == "extend") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "extend",

                    [list](const std::vector<Value>& args,
                           const std::shared_ptr<Environment>&)
                           -> Value {

                        if (args.size() != 1) {
                            throw std::runtime_error("extend expects 1 arg");
                        }

                        list->extend(args[0]);

                        return {};
                    }
                )
            );
        }

        if (attr == "insert") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "insert",

                    [list](const std::vector<Value>& args,
                           const std::shared_ptr<Environment>&)
                           -> Value {

                        if (args.size() != 2) {
                            throw std::runtime_error(
                                "insert expects 2 args"
                            );
                        }

                        list->insert(args[0], args[1]);

                        return Value();
                    }
                )
            );
        }

    }

    throw std::runtime_error("AttributeError: object has no attribute '" +
                            attr.toStdString() + "'");
}

Value getAttrValue(const Value& obj, const QString& attr) {

    // super bypasses __getattribute__
    if (std::holds_alternative<Value::SuperPtr>(obj.data)) {
        return genericGetAttr(obj, attr);
    }

    // instance/class custom __getattribute__
    try {

        Value getattribute = genericGetAttr(obj, "__getattribute__");

        // builtin object.__getattribute__

        bool isDefault = false;

        if (std::holds_alternative<Value::BuiltinFunctionPtr>(getattribute.data)) {

            const auto builtin =
                std::get<Value::BuiltinFunctionPtr>(getattribute.data);

            isDefault = (builtin->name == "__object_getattribute__");
        }

        if (!isDefault) {
            return call(
                getattribute,
                { Value(attr) },
                nullptr
            );
        }

    } catch (const std::runtime_error& e) {

        const std::string msg = e.what();

        if (msg.find("AttributeError") == std::string::npos) {
            throw;
        }
    }

    // default lookup
    try {
        return genericGetAttr(obj, attr);
    }
    catch (const std::runtime_error& e) {

        const std::string msg = e.what();

        // только если attr реально не найден
        if (msg.find("AttributeError") == std::string::npos) {
            throw;
        }
    }

    // __getattr__
    try {

        Value getattr = genericGetAttr(obj, "__getattr__");

        return call(getattr, { Value(attr) }, nullptr);

    } catch (const std::runtime_error& e) {

        const std::string msg = e.what();

        if (msg.find("AttributeError") == std::string::npos) {
            throw;
        }
    }

    throw std::runtime_error(
        "AttributeError: object has no attribute '" +
        attr.toStdString() + "'"
    );
}

Value getAttrFromSuper(const Value::SuperPtr& super, const QString& attr) {
    std::vector<Value::ClassPtr> mro;
    buildMRO(getObjectClass(super->receiver), mro);

    bool foundOrigin = false;

    for (const auto& cls : mro) {

        if (!foundOrigin) {
            if (cls == super->originClass) {
                foundOrigin = true;
            }

            continue;
        }

        if (cls->attributes.contains(attr)) {

            Value val = cls->attributes[attr];

            if (DescriptorUtils::hasGet(val)) {
                return DescriptorUtils::callGet(val, Value(super->receiver), cls);
            }

            return val;
        }
    }

    throw std::runtime_error("AttributeError: object has no attribute '" +
                            attr.toStdString() + "'");
}

void buildMRO(const Value::ClassPtr& cls, std::vector<Value::ClassPtr>& out) {
    out.push_back(cls);

    for (const auto& base : cls->bases) {
        buildMRO(base, out);
    }
}

Value::ClassPtr getObjectClass(const Value& obj)
{
    if (std::holds_alternative<Value::InstancePtr>(obj.data)) {
        return std::get<Value::InstancePtr>(obj.data)->klass;
    }

    if (std::holds_alternative<Value::ClassPtr>(obj.data)) {
        return std::get<Value::ClassPtr>(obj.data);
    }

    throw std::runtime_error("super(): invalid receiver");
}

Value findAttrInHierarchy(const Value::ClassPtr& cls, const QString& attr) {
    if (cls->attributes.contains(attr)) {
        return cls->attributes[attr];
    }

    for (const auto& base : cls->bases) {
        try {
            return findAttrInHierarchy(base, attr);

        } catch (const std::runtime_error& e) {

            const std::string msg = e.what();

            if (msg.find("Attribute not found") == std::string::npos) {
                throw;
            }
        }
    }

    throw std::runtime_error("Attribute not found: " + attr.toStdString());
}

void genericSetAttr(const Value& obj, const QString& attr, const Value& value) {
    // instance
    if (std::holds_alternative<Value::InstancePtr>(obj.data)) {
        const auto instance = std::get<Value::InstancePtr>(obj.data);
        const auto cls = instance->klass;

        // 1. проверяем descriptor в классе
        try {
            const Value descr = findAttrInHierarchy(cls, attr);

            if (DescriptorUtils::hasSet(descr)) {
                DescriptorUtils::callSet(descr, Value(instance), cls, value);
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

void setAttrValue(const Value& obj, const QString& attr, const Value& value) {

    // super bypass
    if (std::holds_alternative<Value::SuperPtr>(obj.data)) {
        genericSetAttr(obj, attr, value);
        return;
    }

    try {

        Value setattr =
            genericGetAttr(obj, "__setattr__");

        bool isDefault = false;

        if (std::holds_alternative<Value::BuiltinFunctionPtr>(
                setattr.data)) {

            const auto builtin =
                std::get<Value::BuiltinFunctionPtr>(
                    setattr.data);

            isDefault =
                (builtin->name == "__object_setattr__");
                }

        if (!isDefault) {

            call(
                setattr,
                {
                    Value(attr),
                    value
                },
                nullptr
            );

            return;
        }

    } catch (const std::runtime_error& e) {

        const std::string msg = e.what();

        if (msg.find("AttributeError")
            == std::string::npos) {
            throw;
            }
    }

    genericSetAttr(obj, attr, value);
}
