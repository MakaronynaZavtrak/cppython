#include "BoundMethod.h"
#include "CallRuntime.h"
#include "ClassMethodValue.h"
#include "ClassUtils.h"
#include "DictValue.h"
#include "Environment.h"
#include "IteratorValue.h"
#include "ListValue.h"
#include "PropertyValue.h"
#include "SetValue.h"
#include "StaticMethodValue.h"
#include "SuperValue.h"
#include "TupleValue.h"
#include "Value.h"
//
// Created by semyo on 04.05.2026.
//
void BuiltinFunction::registerBuiltins(const std::shared_ptr<Environment>& env) {
    env->set("super", Value(std::make_shared<BuiltinFunction>(
    "super",
    [](const std::vector<Value>&, const Kwargs&, const std::shared_ptr<Environment>& local_env) -> Value {

        Value receiver;

        // instance method
       try {
           receiver = local_env->get("self");
       }
       catch (...) {
           // classmethod
           receiver = local_env->get("cls");
       }

        auto clsVal  = local_env->get("__class__");
        auto origin   = std::get<Value::ClassPtr>(clsVal.data);

        return Value(std::make_shared<SuperValue>(
            origin,    // currentClass
            receiver,
            origin     // originClass
        ));
    }
)));

    env->set("hasattr", Value(std::make_shared<BuiltinFunction>(
    "hasattr",
    [](const std::vector<Value>& args,
            const Kwargs&,
            const std::shared_ptr<Environment>&) -> Value {

        if (args.size() != 2)
            throw std::runtime_error("hasattr expects 2 arguments");

        const Value& obj = args[0];
        const QString attr = args[1].asString();

        try {
            genericGetAttr(obj, attr);
            return Value(true);
        } catch (...) {
            return Value(false);
        }
    }
)));

    env->set("getattr", Value(std::make_shared<BuiltinFunction>(
    "getattr",
    [](const std::vector<Value>& args,
            const Kwargs&,
            const std::shared_ptr<Environment>&) -> Value {

        if (args.size() < 2 || args.size() > 3)
            throw std::runtime_error("getattr expects 2 or 3 arguments");

        const Value& obj = args[0];
        const QString& attr = args[1].asString();

        try {
            return genericGetAttr(obj, attr);
        } catch (...) {
            if (args.size() == 3) {
                return args[2]; // default
            }
            throw; // AttributeError
        }
    }
)));

    env->set("setattr", Value(std::make_shared<BuiltinFunction>(
"setattr",
[](const std::vector<Value>& args,
        const Kwargs&,
        const std::shared_ptr<Environment>&) -> Value {

    if (args.size() != 3)
        throw std::runtime_error("setattr expects 3 arguments");

    const Value& obj = args[0];
    const QString attr = args[1].asString();
    const Value& value = args[2];

    setAttrValue(obj, attr, value);
    return {};
}
)));

    env->set("property", Value(std::make_shared<BuiltinFunction>(
    "property",
    [](const std::vector<Value>& args,
            const Kwargs&,
            const std::shared_ptr<Environment>&) -> Value {

        if (args.empty())
            throw std::runtime_error("property needs at least fget");

        auto fget = std::get<Value::FunctionPtr>(args[0].data);

        Value::FunctionPtr fset = nullptr;
        Value::FunctionPtr fdel = nullptr;

        if (args.size() > 1 && !args[1].isNone())
            fset = std::get<Value::FunctionPtr>(args[1].data);

        if (args.size() > 2 && !args[2].isNone())
            fdel = std::get<Value::FunctionPtr>(args[2].data);

        return Value(std::make_shared<PropertyValue>(fget, fset, fdel));
    }
)));

    env->set("__object_getattribute__", Value(std::make_shared<BuiltinFunction>(
        "__object_getattribute__",
        [](const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&) -> Value {

            if (args.size() != 2) {
                throw std::runtime_error("__object_getattribute__ expects 2 arguments");
            }

            return genericGetAttr(args[0], args[1].asString());
    }
)));

    env->set("__object_setattr__",
    Value(std::make_shared<BuiltinFunction>(
        "__object_setattr__",
        [](const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
           -> Value {

            if (args.size() != 3) {
                throw std::runtime_error("__object_setattr__ expects 3 args");
            }

            genericSetAttr(args[0], args[1].asString(), args[2]);

            return {};
        }
)));

    env->set("staticmethod",
    Value(std::make_shared<BuiltinFunction>(
        "staticmethod",
        [](const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
           -> Value {

            if (args.size() != 1) {
                throw std::runtime_error("staticmethod expects 1 argument");
            }

            if (!std::holds_alternative<Value::FunctionPtr>(
                    args[0].data)) {

                throw std::runtime_error("staticmethod expects function");
            }

            return Value(std::make_shared<StaticMethodValue>(std::get<Value::FunctionPtr>(args[0].data)));
        }
)));

    env->set("classmethod",
    Value(std::make_shared<BuiltinFunction>(
        "classmethod",

        [](const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
           -> Value {

            if (args.size() != 1) {
                throw std::runtime_error("classmethod expects 1 argument");
            }

            if (!std::holds_alternative<Value::FunctionPtr>(
                    args[0].data)) {

                throw std::runtime_error("classmethod expects function");
            }

            return Value(
                std::make_shared<ClassMethodValue>(
                    std::get<Value::FunctionPtr>(
                        args[0].data
                    )
                )
            );
        }
)));

    env->set("len",
    Value(std::make_shared<BuiltinFunction>(
        "len",

        [](const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
           -> Value {

            if (args.size() != 1) {
                throw std::runtime_error("len expects 1 arg");
            }

            const Value& obj = args[0];

            // string
            if (const auto s = std::get_if<QString>(&obj.data)) {
                return Value(Value::BigInt(s->size()));
            }

            // list
            if (const auto l = std::get_if<Value::ListPtr>(&obj.data)) {
                return Value(Value::BigInt((*l)->len()));
            }

            try {
                const Value lenMethod =
                getAttrValue(obj, "__len__");

                return call(lenMethod, {}, {}, nullptr);

            } catch (...) {}

            throw std::runtime_error("Object has no len()");

        }
)));

    env->set(
    "iter",
    Value(std::make_shared<BuiltinFunction>(
        "iter",

        [](const std::vector<Value>& args,
           const Kwargs&,
           const std::shared_ptr<Environment>& env)
           -> Value {

            if (args.size() != 1) {
                throw std::runtime_error("iter expects 1 arg");
            }

            const Value method = getAttrValue(args[0], "__iter__");

            return call(method, {}, {}, env);
        }
)));

    env->set(
    "next",

    Value(
        std::make_shared<BuiltinFunction>(
        "next",

        [](const std::vector<Value>& args,
           const Kwargs&,
           const std::shared_ptr<Environment>& env)
           -> Value {

            if (args.size() != 1) {
                throw std::runtime_error("next expects 1 arg");
            }

            const Value method = getAttrValue(args[0], "__next__");

            return call(method, {}, {}, env);
        }
)));

    env->set(
"hash",

Value(
    std::make_shared<BuiltinFunction>(
        "hash",

        [](const std::vector<Value>& args,
           const Kwargs&,
           const std::shared_ptr<Environment>&)
           -> Value {

            if (args.size() != 1) {
                throw std::runtime_error("hash expects 1 arg");
            }

            return Value(
                Value::BigInt(
                    static_cast<long long>(
                        args[0].hash()
                    )
                )
            );
        }
)));

    env->set("list",
Value(std::make_shared<BuiltinFunction>(
    "list",

    [](const std::vector<Value>& args,
       const Kwargs&,
       const std::shared_ptr<Environment>&)
       -> Value {

        if (args.empty()) {
            return Value(std::make_shared<ListValue>());
        }

        if (args.size() != 1) {
            throw std::runtime_error("list expects 0 or 1 argument");
        }

        const Value& iterable = args[0];

        const auto it = iterable.getIterator();

        std::vector<Value> items;

        while (it->hasNext()) {
            items.push_back(it->next());
        }

        return Value(std::make_shared<ListValue>(items));
    }
)));

    env->set("tuple",
Value(std::make_shared<BuiltinFunction>(
    "tuple",

    [](const std::vector<Value>& args,
       const Kwargs&,
       const std::shared_ptr<Environment>&)
       -> Value {

        if (args.empty()) {
            return Value(std::make_shared<TupleValue>(std::vector<Value>{}));
        }

        if (args.size() != 1) {
            throw std::runtime_error("tuple expects 0 or 1 argument");
        }

        const Value& iterable = args[0];

        const auto it = iterable.getIterator();

        std::vector<Value> items;

        while (it->hasNext()) {
            items.push_back(it->next());
        }

        return Value(std::make_shared<TupleValue>(items));
    }
)));

    env->set("set",
Value(std::make_shared<BuiltinFunction>(
    "set",

    [](const std::vector<Value>& args,
       const Kwargs&,
       const std::shared_ptr<Environment>&)
       -> Value {

        if (args.empty()) {
            return Value(std::make_shared<SetValue>());
        }

        if (args.size() != 1) {
            throw std::runtime_error("set expects 0 or 1 argument");
        }

        const Value& iterable = args[0];

        const auto it = iterable.getIterator();

        const auto set = std::make_shared<SetValue>();

        while (it->hasNext()) {
            set->add(it->next());
        }

        return Value(set);
    }
)));

    env->set("dict",
Value(std::make_shared<BuiltinFunction>(
    "dict",

    [](const std::vector<Value>& args,
       const Kwargs&,
       const std::shared_ptr<Environment>&)
       -> Value {

        if (args.empty()) {
            return Value(std::make_shared<DictValue>());
        }

        if (args.size() != 1) {
            throw std::runtime_error("dict expects 0 or 1 argument");
        }

        const Value& iterable = args[0];

        const auto it = iterable.getIterator();

        auto dict = std::make_shared<DictValue>();

        while (it->hasNext()) {

            Value item = it->next();

            if (!item.isTuple()) {
                throw std::runtime_error("dict() expects iterable of pairs");
            }

            const auto tup = item.asTuple();

            if (tup->items.size() != 2) {
                throw std::runtime_error("dict() expects (key, value) pairs");
            }

            dict->setItem(tup->items[0], tup->items[1]);
        }

        return Value(dict);
    }
)));

}

Value BuiltinFunction::get(const Value::InstancePtr& instance, const Value::ClassPtr& owner) {

    if (!instance) {
        return Value(shared_from_this());
    }

    return Value(std::make_shared<BoundMethod>(
        Value(shared_from_this()),
        Value(instance),
        owner
    ));
}
