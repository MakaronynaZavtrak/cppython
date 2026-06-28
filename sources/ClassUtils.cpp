//
// Created by semyo on 03.05.2026.
//
#include "ClassUtils.h"

#include "CallRuntime.h"
#include "ClassValue.h"
#include "DescriptorUtils.h"
#include "../runtime/builtins/dict/DictMethods.h"
#include "InstanceValue.h"
#include "../runtime/builtins/iterator/IteratorMethods.h"
#include "../runtime/builtins/list/ListMethods.h"
#include "ListValue.h"
#include "../runtime/builtins/set/SetMethods.h"
#include "../runtime/builtins/str/StrMethods.h"
#include "SuperValue.h"
#include "../exception/AttributeErrorException.h"
#include "../exception/TypeErrorException.h"
#include "../runtime/builtins/bytearray/ByteArrayMethods.h"
#include "../runtime/builtins/bytes/BytesMethods.h"
#include "../runtime/builtins/frozenset/FrozenSetMethods.h"
#include "../runtime/builtins/tuple/TupleMethods.h"

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
    if (obj.isInstance()) {

        auto instance = obj.asInstance();
        auto cls = instance->klass;

        //  1. data descriptor (hasSet)
        try {
            if (Value val = findAttrInHierarchy(cls, attr);
                DescriptorUtils::hasGet(val) && DescriptorUtils::hasSet(val)) {

                return DescriptorUtils::callGet(val, Value(instance), cls);
            }

        } catch (const AttributeErrorException& e) {}

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

        } catch (const AttributeErrorException& e) {}
    }

    // super
    if (obj.isSuper()) {
        return getAttrFromSuper(obj.asSuper(), attr);
    }

    // class
    if (obj.isClass()) {

        auto cls = obj.asClass();

        Value val = findAttrInHierarchy(cls, attr);

        if (DescriptorUtils::hasGet(val)) {
            return DescriptorUtils::callGet(val, Value(), cls);
        }

        return val;
    }

    if (obj.isList()) {
        return getListAttr(obj, attr);
    }

    if (obj.isDict()) {
        return getDictAttr(obj, attr);
    }

    if (obj.isDictKeysView()) {

        if (attr == "__iter__") {
            return makeIterMethod(obj);
        }

    }

    if (obj.isDictValuesView()) {

        if (attr == "__iter__") {
            return makeIterMethod(obj);
        }

    }

    if (obj.isDictItemsView()) {

        if (attr == "__iter__") {
            return makeIterMethod(obj);
        }
    }

    if (obj.isTuple()) {
        return getTupleAttr(obj, attr);
    }

    if (obj.isSet()) {
        return getSetAttr(obj, attr);
    }


    if (std::holds_alternative<Value::IteratorPtr>(obj.data)) {
        return getIteratorAttr(obj, attr);
    }

    if (obj.isString()) {
        return getStrAttr(obj, attr);
    }

    if (obj.isBytes()) {
        return getBytesAttr(obj, attr);
    }

    if (obj.isByteArray()) {
        return getByteArrayAttr(obj, attr);
    }

    if (obj.isFrozenSet()) {
        return getFrozenSetAttr(obj, attr);
    }

    throw AttributeErrorException(
        "object has no attribute '" + attr + "'"
    );
}

Value makeIterMethod(const Value& obj) {

    return Value(
        std::make_shared<BuiltinFunction>(
            "__iter__",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
                  -> Value {

                if (!args.empty()) {
                    throw TypeErrorException(
    "__iter__() takes 0 positional arguments but "
    + QString::number(args.size()) + " were given"
);
                }

                return Value(obj.getIterator());
            }
        )
    );
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
            return call(getattribute, { Value(attr) }, {}, nullptr);
        }

    } catch (const AttributeErrorException& e) {}

    // default lookup
    try {
        return genericGetAttr(obj, attr);
    }
    catch (const AttributeErrorException& e) {}

    // __getattr__
    try {

        Value getattr = genericGetAttr(obj, "__getattr__");

        return call(getattr, { Value(attr) }, {}, nullptr);

    } catch (const AttributeErrorException& e) {}

    throw AttributeErrorException("object has no attribute '" + attr + "'");
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

    throw AttributeErrorException("object has no attribute '" + attr + "'");
}

void buildMRO(const Value::ClassPtr& cls, std::vector<Value::ClassPtr>& out) {
    out.push_back(cls);

    for (const auto& base : cls->bases) {
        buildMRO(base, out);
    }
}

Value::ClassPtr getObjectClass(const Value& obj) {

    if (obj.isInstance()) {
        return obj.asInstance()->klass;
    }

    if (std::holds_alternative<Value::ClassPtr>(obj.data)) {
        return std::get<Value::ClassPtr>(obj.data);
    }

    throw TypeErrorException("super(): invalid receiver");
}

Value findAttrInHierarchy(const Value::ClassPtr& cls, const QString& attr) {

    if (cls->attributes.contains(attr)) {
        return cls->attributes[attr];
    }

    for (const auto& base : cls->bases) {

        try {

            return findAttrInHierarchy(base, attr);

        } catch (const AttributeErrorException& e) {}
    }

    throw AttributeErrorException("Attribute not found: " + attr);
}

void genericSetAttr(const Value& obj, const QString& attr, const Value& value) {
    // instance
    if (obj.isInstance()) {

        const auto instance = obj.asInstance();
        const auto cls = instance->klass;

        // 1. проверяем descriptor в классе
        try {
            if (const Value descr = findAttrInHierarchy(cls, attr);
                DescriptorUtils::hasSet(descr)) {

                DescriptorUtils::callSet(descr, Value(instance), cls, value);
                return;
            }

        } catch (const AttributeErrorException& e) {}

        // 2. обычная запись в поля
        instance->fields[attr] = value;
        return;
    }

    // class
    if (obj.isClass()) {
        const auto cls = obj.asClass();
        cls->attributes[attr] = value;
        return;
    }

    throw AttributeErrorException("object has no attributes");
}

void setAttrValue(const Value& obj, const QString& attr, const Value& value) {

    // super bypass
    if (obj.isSuper()) {
        genericSetAttr(obj, attr, value);
        return;
    }

    try {

        const Value setattr = genericGetAttr(obj, "__setattr__");

        bool isDefault = false;

        if (setattr.isBuiltinFunction()) {

            const auto builtin = setattr.asBuiltinFunction();

            isDefault = builtin->name == "__object_setattr__";
        }

        if (!isDefault) {

            call(setattr, { Value(attr), value }, {}, nullptr);

            return;
        }

    } catch (const AttributeErrorException& e) {}

    genericSetAttr(obj, attr, value);
}
