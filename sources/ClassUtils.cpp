//
// Created by semyo on 03.05.2026.
//
#include "ClassUtils.h"

#include "CallRuntime.h"
#include "ClassValue.h"
#include "DescriptorUtils.h"
#include "DictValue.h"
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
#include "../runtime/builtins/range/RangeMethods.h"
#include "../runtime/builtins/tuple/TupleMethods.h"
#include "ModuleValue.h"
#include "PartialValue.h"
#include "TupleValue.h"

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

        if (attr == "__name__") {
            return Value(cls->name);
        }

        try {
            Value val = findAttrInHierarchy(cls, attr);

            if (DescriptorUtils::hasGet(val)) {
                return DescriptorUtils::callGet(val, Value(), cls);
            }

            return val;
        } catch (const AttributeErrorException&) {

            // Атрибут не найден в самом классе — ищем в метаклассе
            // (методы/атрибуты метакласса, привязанные к cls как получателю).
            // Только для реального кастомного метакласса, чтобы дефолтные
            // классы вели себя ровно как раньше.
            if (cls->metaclass && cls->metaclass != Runtime::typeClass) {

                Value val = findAttrInHierarchy(cls->metaclass, attr);

                if (DescriptorUtils::hasGet(val)) {
                    return DescriptorUtils::callGet(val, Value(cls), cls->metaclass);
                }

                return val;
            }

            throw;
        }
    }

    if (obj.isModule()) {

        const auto mod = obj.asModule();

        if (mod->members.contains(attr)) {
            return mod->members[attr];
        }

        throw AttributeErrorException(
            "module '" + mod->name + "' has no attribute '" + attr + "'"
        );
    }

    if (obj.isPartial()) {

        const auto p = obj.asPartial();

        if (attr == "func") {
            return p->func;
        }

        if (attr == "args") {
            return Value(std::make_shared<TupleValue>(p->args));
        }

        if (attr == "keywords") {
            const auto dict = std::make_shared<DictValue>();
            for (const auto &[key, value] : p->keywords) {
                dict->setItem(Value(key), value);
            }
            return Value(dict);
        }

        throw AttributeErrorException(
            "'functools.partial' object has no attribute '" + attr + "'"
        );
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

    if (obj.isRange()) {
        return getRangeAttr(obj, attr);
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

QString pythonStr(const Value& obj) {

    if (obj.isInstance()) {
        try {
            const Value m = getAttrValue(obj, "__str__");
            const Value r = call(m, {}, {}, nullptr);

            if (!r.isString()) {
                throw TypeErrorException("__str__ returned non-string");
            }
            return r.toString();

        } catch (const AttributeErrorException&) {}
    }

    return obj.toString();
}

// Возвращает class-объект (тип) значения — основа для type(x) и __class__.
Value typeOf(const Value& obj) {
    if (obj.isInstance())  return Value(obj.asInstance()->klass);
    if (obj.isClass()) {
        const auto& c = obj.asClass();
        return Value(c->metaclass ? c->metaclass : Runtime::typeClass);
    }

    if (obj.isBool())      return Value(Runtime::boolClass);
    if (obj.isBigInt())    return Value(Runtime::intClass);
    if (obj.isBigFloat())  return Value(Runtime::floatClass);
    if (obj.isString())    return Value(Runtime::strClass);
    if (obj.isBytes())     return Value(Runtime::bytesClass);
    if (obj.isByteArray()) return Value(Runtime::bytearrayClass);
    if (obj.isList())      return Value(Runtime::listClass);
    if (obj.isTuple())     return Value(Runtime::tupleClass);
    if (obj.isDict())      return Value(Runtime::dictClass);
    if (obj.isSet())       return Value(Runtime::setClass);
    if (obj.isFrozenSet()) return Value(Runtime::frozensetClass);
    if (obj.isRange())     return Value(Runtime::rangeClass);
    if (obj.isNone())      return Value(Runtime::noneTypeClass);

    if (obj.isBuiltinFunction()) return Value(Runtime::builtinFunctionClass);
    if (obj.isCallable())        return Value(Runtime::functionClass);

    return Value(Runtime::objectClass);
}

Value getAttrValue(const Value& obj, const QString& attr) {

    if (attr == "__class__") {
        return typeOf(obj);
    }

    // super bypasses __getattribute__
    if (obj.isSuper()) {
        return genericGetAttr(obj, attr);
    }

    // instance/class custom __getattribute__
    try {

        Value getattribute = genericGetAttr(obj, "__getattribute__");

        // builtin object.__getattribute__

        bool isDefault = false;

        if (getattribute.isBuiltinFunction()) {

            const auto builtin = getattribute.asBuiltinFunction();

            isDefault = builtin->name == "__object_getattribute__";
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

            // __new__ по семантике Python — staticmethod: не привязываем его
            // через super, иначе super().__new__(mcs, ...) получит лишний
            // receiver и уедет на один аргумент вправо.
            if (attr == "__new__") {
                return val;
            }

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
