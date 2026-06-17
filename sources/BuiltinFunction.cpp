#include "BoundMethod.h"
#include "ByteArrayValue.h"
#include "BytesValue.h"
#include "CallRuntime.h"
#include "ClassMethodValue.h"
#include "ClassUtils.h"
#include "DictValue.h"
#include "Environment.h"
#include "FrozenSetValue.h"
#include "IteratorValue.h"
#include "ListValue.h"
#include "PropertyValue.h"
#include "SetValue.h"
#include "StaticMethodValue.h"
#include "StrValue.h"
#include "SuperValue.h"
#include "TupleValue.h"
#include "Value.h"
#include "../runtime/ArgValidation.h"
#include "../runtime/RuntimeUtils.h"
//
// Created by semyo on 04.05.2026.
//

const Value* findKwarg(const Kwargs& kwargs, const QString& name) {

    const auto it = std::find_if(kwargs.begin(), kwargs.end(),
        [&](const auto& pair) { return pair.first == name; });

    if (it == kwargs.end()) {
        return nullptr;
    }

    return &it->second;
}


void BuiltinFunction::registerBuiltins(const std::shared_ptr<Environment> &env) {

    env->set("super",
             makeBuiltin(
                 "super",

                 [](const std::vector<Value> &,
                    const Kwargs &,
                    const std::shared_ptr<Environment> &local_env) -> Value {

                     Value receiver;

                     // instance method
                     try {
                         receiver = local_env->get("self");
                     } catch (...) {
                         // classmethod
                         receiver = local_env->get("cls");
                     }

                     auto clsVal = local_env->get("__class__");
                     auto origin = std::get<Value::ClassPtr>(clsVal.data);

                     return Value(std::make_shared<SuperValue>(
                         origin, // currentClass
                         receiver,
                         origin // originClass
                     ));
                 }
             ));

    env->set("hasattr",
             makeBuiltin(
                 "hasattr",

                 [](const std::vector<Value> &args,
                    const Kwargs &,
                    const std::shared_ptr<Environment> &) -> Value {

                     expectArgs(args, 2, "hasattr");

                     const Value &obj = args[0];
                     const QString attr = args[1].asString()->toString();

                     try {
                         genericGetAttr(obj, attr);

                         return Value(true);
                     } catch (...) {
                         return Value(false);
                     }
                 }
             ));

    env->set("getattr",
             makeBuiltin(
                 "getattr",

                 [](const std::vector<Value> &args,
                    const Kwargs &,
                    const std::shared_ptr<Environment> &) -> Value {

                     expectArgsRange(args, 2, 3, "getattr");

                     const Value &obj = args[0];
                     const QString &attr = args[1].asString()->toString();

                     try {
                         return genericGetAttr(obj, attr);
                     } catch (...) {
                         if (args.size() == 3) {
                             return args[2]; // default
                         }
                         throw; // AttributeError
                     }
                 }
             ));

    env->set("setattr",
             makeBuiltin(
                 "setattr",

                 [](const std::vector<Value> &args,
                    const Kwargs &,
                    const std::shared_ptr<Environment> &) -> Value {

                     expectArgs(args, 3, "setattr");

                     const Value &obj = args[0];
                     const QString attr = args[1].asString()->toString();
                     const Value &value = args[2];

                     setAttrValue(obj, attr, value);
                     return {};
                 }
             ));

    env->set("property",
             makeBuiltin(
                 "property",

                 [](const std::vector<Value> &args,
                    const Kwargs &,
                    const std::shared_ptr<Environment> &) -> Value {

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
             ));

    env->set("__object_getattribute__",
             makeBuiltin(
                 "__object_getattribute__",

                 [](const std::vector<Value> &args,
                    const Kwargs &,
                    const std::shared_ptr<Environment> &) -> Value {

                     expectArgs(args, 2, "__object_getattribute__");

                     return genericGetAttr(args[0], args[1].asString()->toString());
                 }
             ));

    env->set("__object_setattr__",
             makeBuiltin(
                 "__object_setattr__",

                 [](const std::vector<Value> &args,
                    const Kwargs &,
                    const std::shared_ptr<Environment> &) -> Value {

                     expectArgs(args, 3, "__object_setattr__");

                     genericSetAttr(args[0], args[1].asString()->toString(), args[2]);

                     return {};
                 }
             ));

    env->set("staticmethod",
             makeBuiltin(
                 "staticmethod",

                 [](const std::vector<Value> &args,
                    const Kwargs &,
                    const std::shared_ptr<Environment> &) -> Value {

                     expectArgs(args, 1, "staticmethod");

                     if (!std::holds_alternative<Value::FunctionPtr>(args[0].data)) {
                         throw std::runtime_error("staticmethod expects function");
                     }

                     return Value(
                         std::make_shared<StaticMethodValue>(
                             std::get<Value::FunctionPtr>(
                                 args[0].data
                            )
                        )
                    );
                 }
             ));

    env->set("classmethod",
             makeBuiltin(
                 "classmethod",

                 [](const std::vector<Value> &args,
                    const Kwargs &,
                    const std::shared_ptr<Environment> &) -> Value {

                     expectArgs(args, 1, "classmethod");

                     if (!std::holds_alternative<Value::FunctionPtr>(args[0].data)) {
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
             ));

    env->set("len",
             makeBuiltin(
                 "len",

                 [](const std::vector<Value> &args,
                    const Kwargs &,
                    const std::shared_ptr<Environment> &) -> Value {

                     expectArgs(args, 1, "len");

                     const Value &obj = args[0];

                     // string
                     if (const auto s = std::get_if<Value::StrPtr>(&obj.data)) {
                         return Value(Value::BigInt(s->get()->len()));
                     }

                     // list
                     if (const auto l = std::get_if<Value::ListPtr>(&obj.data)) {
                         return Value(Value::BigInt((*l)->len()));
                     }

                     try {

                         const Value lenMethod = getAttrValue(obj, "__len__");

                         return call(lenMethod, {}, {}, nullptr);

                     } catch (...) {}

                     throw std::runtime_error("Object has no len()");
                 }
             ));

    env->set("iter",
        makeBuiltin(
            "iter",

            [](const std::vector<Value> &args,
               const Kwargs &,
               const std::shared_ptr<Environment> &env) -> Value {

                expectArgs(args, 1, "iter");

                const Value method = getAttrValue(args[0], "__iter__");

                return call(method, {}, {}, env);
            }
        ));

    env->set("next",
        makeBuiltin(
            "next",

            [](const std::vector<Value> &args,
               const Kwargs &,
               const std::shared_ptr<Environment> &env) -> Value {

                expectArgs(args, 1, "next");

                const Value method = getAttrValue(args[0], "__next__");

                return call(method, {}, {}, env);
            }
        ));

    env->set("hash",
        makeBuiltin(
            "hash",

            [](const std::vector<Value> &args,
               const Kwargs &,
               const std::shared_ptr<Environment> &) -> Value {

                expectArgs(args, 1, "hash");

                return Value(
                    Value::BigInt(
                        static_cast<long long>(
                            args[0].hash()
                        )
                    )
                );
            }
        ));

    env->set("list",
             makeBuiltin(
                 "list",

                 [](const std::vector<Value> &args,
                    const Kwargs &,
                    const std::shared_ptr<Environment> &) -> Value {

                     expectArgsRange(args, 0, 1, "list");

                     if (args.empty()) {
                         return Value(std::make_shared<ListValue>());
                     }

                     const Value &iterable = args[0];

                     const auto it = iterable.getIterator();

                     std::vector<Value> items;

                     while (it->hasNext()) {
                         items.push_back(it->next());
                     }

                     return Value(std::make_shared<ListValue>(items));
                 }
             ));

    env->set("tuple",
             makeBuiltin(
                 "tuple",

                 [](const std::vector<Value> &args,
                    const Kwargs &,
                    const std::shared_ptr<Environment> &) -> Value {

                     expectArgsRange(args, 0, 1, "tuple");

                     if (args.empty()) {
                         return Value(std::make_shared<TupleValue>(std::vector<Value>{}));
                     }

                     const Value &iterable = args[0];

                     const auto it = iterable.getIterator();

                     std::vector<Value> items;

                     while (it->hasNext()) {
                         items.push_back(it->next());
                     }

                     return Value(std::make_shared<TupleValue>(items));
                 }
             ));

    env->set("set",
             makeBuiltin(
                 "set",

                 [](const std::vector<Value> &args,
                    const Kwargs &,
                    const std::shared_ptr<Environment> &) -> Value {

                     expectArgsRange(args, 0, 1, "set");

                     if (args.empty()) {
                         return Value(std::make_shared<SetValue>());
                     }

                     const Value &iterable = args[0];

                     const auto it = iterable.getIterator();

                     const auto set = std::make_shared<SetValue>();

                     while (it->hasNext()) {
                         set->add(it->next());
                     }

                     return Value(set);
                 }
             ));

    env->set("dict",
             makeBuiltin(
                 "dict",

                 [](const std::vector<Value> &args,
                    const Kwargs &,
                    const std::shared_ptr<Environment> &) -> Value {

                     expectArgsRange(args, 0, 1, "dict");

                     if (args.empty()) {
                         return Value(std::make_shared<DictValue>());
                     }

                     const Value &iterable = args[0];

                     const auto it = iterable.getIterator();

                     const auto dict = std::make_shared<DictValue>();

                     while (it->hasNext()) {

                         Value item = it->next();

                         const auto tup = item.asTuple("dict()");

                         if (tup->items.size() != 2) {
                             throw std::runtime_error("dict() expects (key, value) pairs");
                         }

                         dict->setItem(tup->items[0], tup->items[1]);
                     }

                     return Value(dict);
                 }
             ));

    env->set(

        "frozenset",

        makeBuiltin(

            "frozenset",

            [](const std::vector<Value> &args,
               const Kwargs &,
               const std::shared_ptr<Environment> &) -> Value {
                expectArgsRange(args, 0, 1, "frozenset");

                if (args.empty()) {
                    return Value(
                        std::make_shared<FrozenSetValue>()
                    );
                }

                const auto iterator =
                        args[0].getIterator();

                QSet<Value> elements;

                while (iterator->hasNext()) {
                    elements.insert(
                        iterator->next()
                    );
                }

                return Value(
                    std::make_shared<FrozenSetValue>(
                        std::move(elements)
                    )
                );
            }
        )
    );

    env->set("repr",
             makeBuiltin(
                 "repr",

                 [](const std::vector<Value> &args,
                    const Kwargs &,
                    const std::shared_ptr<Environment> &) -> Value {

                     expectArgs(args, 1, "repr");

                     return Value(args[0].repr());
                 }
             ));

    env->set(
    "print",
    makeBuiltin(
        "print",

        [](const std::vector<Value>& args,
           const Kwargs& kwargs,
           const std::shared_ptr<Environment>&) -> Value {

            QString sep = " ";
            QString end = "\n";
            bool flush = false;

            // sep
            if (const auto sepArg = findKwarg(kwargs, "sep"))
            {
                if (!sepArg->isString()) {
                    throw std::runtime_error("TypeError: sep must be str");
                }

                sep = sepArg->toString();
            }

            // end
            if (const auto endArg = findKwarg(kwargs, "end"))
            {
                if (!endArg->isString()) {
                    throw std::runtime_error("TypeError: end must be str");
                }

                end = endArg->toString();
            }

            // flush
            if (const auto flushArg = findKwarg(kwargs, "flush")) {
                flush = flushArg->toBool();
            }

            // output
            for (size_t i = 0; i < args.size(); ++i) {

                if (i > 0) {
                    std::cout << sep.toStdString();
                }

                std::cout << args[i].toString().toStdString();
            }

            std::cout << end.toStdString();

            if (flush) {
                std::cout.flush();
            }

            return {}; // None
        }
    ));

    env->set("__str_call__",
             makeBuiltin(
                 "__str_call__",

                 [](const std::vector<Value> &args,
                    const Kwargs &,
                    const std::shared_ptr<Environment> &) -> Value {

                     if (args.empty()) {
                         return Value("");
                     }

                     return Value(args[0].toString());
                 }
             ));

    env->set("__bytes_call__",
    makeBuiltin(
        "__bytes_call__",

        [](const std::vector<Value>& args,
           const Kwargs&,
           const std::shared_ptr<Environment>&) -> Value {

            if (args.empty()) {
                return Value(
                    std::make_shared<BytesValue>(
                        QByteArray()
                    )
                );
            }

            const Value& obj = args[0];

            try {

                Value method = getAttrValue(obj, "__bytes__");

                Value result = call(method, {}, {}, nullptr);

                if (!result.isBytes()) {
                    throw std::runtime_error(
                        "TypeError: __bytes__ returned non-bytes"
                    );
                }

                return result;

            } catch (const std::runtime_error &e) {

                if (const std::string msg = e.what();
                    msg.find("AttributeError") == std::string::npos) {
                    throw;
                }
            }

            if (obj.isBytes()) {
                return obj;
            }

            if (obj.isString()) {

                return Value(
                    std::make_shared<BytesValue>(
                        obj.toString().toUtf8()
                    )
                );
            }

            throw std::runtime_error(
                "TypeError: cannot convert object to bytes"
            );
        }
    ));

    env->set("__bytearray_call__",

     makeBuiltin(
         "__bytearray_call__",

         [](const std::vector<Value> &args,
            const Kwargs &,
            const std::shared_ptr<Environment> &) -> Value {
             expectArgsRange(args, 0, 1, "bytearray");

             if (args.empty()) {
                 return Value(
                     std::make_shared<ByteArrayValue>(
                         QByteArray()
                     )
                 );
             }

             const Value &obj = args[0];

             try {

                Value method = getAttrValue(obj, "__bytes__");

                Value result = call(method, {}, {}, nullptr);

                if (!result.isBytes()) {
                    throw std::runtime_error(
                        "TypeError: __bytes__ returned non-bytes"
                    );
                }

                return result;

            } catch (const std::runtime_error &e) {

                if (const std::string msg = e.what();
                    msg.find("AttributeError") == std::string::npos) {
                    throw;
                }
            }

             if (obj.isByteArray()) {
                 return Value(
                     std::make_shared<ByteArrayValue>(
                         obj.asByteArray()->bytes()
                     )
                 );
             }

             if (obj.isBytes()) {
                 return Value(
                     std::make_shared<ByteArrayValue>(
                         obj.asBytes()->bytes()
                     )
                 );
             }

             if (obj.isString()) {
                 return Value(
                     std::make_shared<ByteArrayValue>(
                         obj.toString().toUtf8()
                     )
                 );
             }

             throw std::runtime_error(
                 "TypeError: cannot convert object to bytearray"
             );
         }
    ));

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

QString BuiltinFunction::toString() const {
    return "<built-in function " + name + ">";
}
