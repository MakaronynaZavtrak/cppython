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
#include "MapIterator.h"
#include "PropertyValue.h"
#include "ReversedSequenceIterator.h"
#include "SetValue.h"
#include "StaticMethodValue.h"
#include "StrValue.h"
#include "SuperValue.h"
#include "TupleValue.h"
#include "Value.h"
#include "../exception/AttributeErrorException.h"
#include "../exception/StopIterationException.h"
#include "../exception/TypeErrorException.h"
#include "../exception/ValueErrorException.h"
#include "../runtime/ArgValidation.h"
#include "../runtime/RuntimeUtils.h"
//
// Created by semyo on 04.05.2026.
//

Value::BigInt parseIntLiteral(const QString& raw, int base) {

    auto fail = [&]() {
        throw ValueErrorException(
            "invalid literal for int() with base " +
            QString::number(base == 0 ? 10 : base) + ": '" + raw + "'"
        );
    };

    QString s = raw.trimmed();
    s.remove('_');

    if (s.isEmpty()) fail();

    bool negative = false;
    if (s.startsWith('+'))      s = s.mid(1);
    else if (s.startsWith('-')) { negative = true; s = s.mid(1); }

    if (base == 0) {
        if      (s.startsWith("0x", Qt::CaseInsensitive)) { base = 16; s = s.mid(2); }
        else if (s.startsWith("0b", Qt::CaseInsensitive)) { base = 2;  s = s.mid(2); }
        else if (s.startsWith("0o", Qt::CaseInsensitive)) { base = 8;  s = s.mid(2); }
        else base = 10;
    }
    else if (base == 16 && s.startsWith("0x", Qt::CaseInsensitive)) s = s.mid(2);
    else if (base == 2  && s.startsWith("0b", Qt::CaseInsensitive)) s = s.mid(2);
    else if (base == 8  && s.startsWith("0o", Qt::CaseInsensitive)) s = s.mid(2);

    if (s.isEmpty()) fail();

    Value::BigInt result = 0;

    for (const QChar& c : s) {

        int digit;

        if (c.isDigit())       digit = c.unicode() - '0';
        else if (c.isLetter()) digit = c.toLower().unicode() - 'a' + 10;
        else                   { fail(); digit = 0; }

        if (digit >= base) fail();

        result = result * base + digit;
    }

    return negative ? -result : result;
}

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
                     auto origin = clsVal.asClass();

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
                         throw TypeErrorException("property needs at least fget");

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

                     if (!args[0].isFunction()) {
                         throw TypeErrorException("staticmethod expects function");
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

                     if (!args[0].isFunction()) {
                         throw TypeErrorException("classmethod expects function");
                     }

                     return Value(
                         std::make_shared<ClassMethodValue>(
                             args[0].asFunction()
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

                     throw AttributeErrorException("Object has no len()");
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

                     while (true) {
                     try {
                         items.push_back(it->next());
                     }
                     catch (const StopIterationException&) {
                         break;
                     }
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

                     while (true) {
                     try {
                         items.push_back(it->next());
                     }
                     catch (const StopIterationException&) {
                         break;
                     }
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

                     while (true) {
                     try {
                         set->add(it->next());
                     }
                     catch (const StopIterationException&) {
                         break;
                     }
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

                     while (true) {

                         try {
                             Value item = it->next();

                             const auto tup = item.asTuple("dict()");

                             if (tup->items.size() != 2) {
                                 throw TypeErrorException("dict() expects (key, value) pairs");
                             }

                             dict->setItem(tup->items[0], tup->items[1]);
                         } catch (StopIterationException) {
                             break;
                         }
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

                const auto iterator = args[0].getIterator();

                QSet<Value> elements;

                while (true) {
                    try {
                        elements.insert(
                            iterator->next()
                        );
                    } catch (StopIterationException) {
                        break;
                    }
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
            if (const auto sepArg = findKwarg(kwargs, "sep")) {
                if (!sepArg->isString()) {
                    throw TypeErrorException("sep must be str");
                }

                sep = sepArg->toString();
            }

            // end
            if (const auto endArg = findKwarg(kwargs, "end"))
            {
                if (!endArg->isString()) {
                    throw TypeErrorException("end must be str");
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

                std::cout << pythonStr(args[i]).toStdString();
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
                     expectArgsRange(args, 0, 1, "str");

                     if (args.empty()) {
                         return Value("");
                     }

                     return Value(pythonStr(args[0]));
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
                    throw TypeErrorException("__bytes__ returned non-bytes");
                }

                return result;

            } catch ([[maybe_unused]] AttributeErrorException &e) {}

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

            throw TypeErrorException("cannot convert object to bytes");
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
                    throw TypeErrorException("__bytes__ returned non-bytes");
                }

                return result;

            } catch ([[maybe_unused]] const AttributeErrorException& e) {}

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

             throw TypeErrorException("cannot convert object to bytearray");
         }
    ));

    env->set(
    "reversed",

    makeBuiltin(
        "reversed",

        [](const std::vector<Value>& args,
           const Kwargs&,
           const std::shared_ptr<Environment>& env) -> Value {

            expectArgs(args, 1, "reversed");

            const Value& obj = args[0];

            // 1. __reversed__
            try {

                Value method = getAttrValue(obj, "__reversed__");

                return call(method, {}, {}, env);

            } catch (...) {}

            // 2. fallback через __len__ + __getitem__
            try {

                Value lenMethod = getAttrValue(obj, "__len__");

                Value getItemMethod = getAttrValue(obj, "__getitem__");

                Value lenValue = call(lenMethod, {}, {}, env);

                auto len = static_cast<std::ptrdiff_t>(lenValue.toBigInt());

                return Value(std::make_shared<ReversedSequenceIterator>(
                        obj, len
                    )
                );

            } catch (...) {}

            throw TypeErrorException(
                obj.toString() + "' object is not reversible"
            );
        }
    )
);

    env->set(
    "format",

    makeBuiltin(
        "format",

        [](const std::vector<Value>& args,
           const Kwargs&,
           const std::shared_ptr<Environment>& env) -> Value {

            expectArgsRange(args, 1, 2, "format");

            if (!args[1].isString()) {
                throw TypeErrorException("format() argument 2 must be str");
            }

            const Value& obj = args[0];

            QString formatSpec;

            if (args.size() == 2) {

                formatSpec = args[1].toString();
            }

            const Value method = getAttrValue(obj, "__format__");

            return call(
                method,
                {
                    Value(formatSpec)
                },
                {},
                env
            );
        }
    )
);

    env->set("int",
         makeBuiltin(
             "int",

             [](const std::vector<Value> &args,
                const Kwargs &,
                const std::shared_ptr<Environment> &) -> Value {

                 expectArgsRange(args, 0, 2, "int");

                 if (args.empty()) {
                     return Value(Value::BigInt(0));
                 }

                 const Value &obj = args[0];

                 // int(x, base) — только для строк
                 if (args.size() == 2) {

                     if (!obj.isString()) {
                         throw TypeErrorException(
                             "int() can't convert non-string with explicit base"
                         );
                     }

                     const auto base = args[1].toBigInt().convert_to<int>();

                     if (base != 0 && (base < 2 || base > 36)) {
                         throw ValueErrorException("int() base must be >= 2 and <= 36, or 0");
                     }

                     return Value(parseIntLiteral(obj.asString()->toString(), base));
                 }

                 if (obj.isBool()) {
                     return Value(Value::BigInt(obj.toBool() ? 1 : 0));
                 }

                 if (obj.isBigInt()) {
                     return obj;
                 }

                 // усечение К НУЛЮ: int(-5.7) == -5, не -6
                 if (obj.isBigFloat()) {
                     return Value(obj.asBigFloat().convert_to<Value::BigInt>());
                 }

                 if (obj.isString()) {
                     return Value(parseIntLiteral(obj.asString()->toString(), 10));
                 }

                 try {
                     Value method = getAttrValue(obj, "__int__");
                     Value result = call(method, {}, {}, nullptr);

                     if (!result.isBigInt()) {
                         throw TypeErrorException("__int__ returned non-int");
                     }

                     return result;
                 }
                 catch (const AttributeErrorException &) {}

                 throw TypeErrorException(
                     "int() argument must be a string, a bytes-like object or a real number, not '"
                     + obj.getTypeName() + "'"
                 );
             }
         ));

    env->set("bool",
         makeBuiltin(
             "bool",

             [](const std::vector<Value> &args,
                const Kwargs &,
                const std::shared_ptr<Environment> &) -> Value {

                 expectArgsRange(args, 0, 1, "bool");

                 if (args.empty()) {
                     return Value(false);
                 }

                 return Value(args[0].toBool());
             }
         ));

    env->set("float",
         makeBuiltin(
             "float",

             [](const std::vector<Value> &args,
                const Kwargs &,
                const std::shared_ptr<Environment> &) -> Value {

                 expectArgsRange(args, 0, 1, "float");

                 if (args.empty()) {
                     return Value(Value::BigFloat(0));
                 }

                 const Value &obj = args[0];

                 if (obj.isNumeric()) {
                     return Value(obj.toBigFloat());
                 }

                 if (obj.isString()) {

                     QString s = obj.asString()->toString().trimmed();
                     s.remove('_');

                     try {
                         return Value(Value::BigFloat(s.toStdString()));
                     }
                     catch (...) {
                         throw ValueErrorException(
                             "could not convert string to float: '" +
                             obj.asString()->toString() + "'"
                         );
                     }
                 }

                 try {
                     const Value method = getAttrValue(obj, "__float__");
                     return call(method, {}, {}, nullptr);
                 }
                 catch (const AttributeErrorException &) {}

                 throw TypeErrorException(
                     "float() argument must be a string or a real number, not '" + obj.getTypeName() + "'"
                 );
             }
         ));

    env->set("sum",
    makeBuiltin(
        "sum",

        [](const std::vector<Value> &args,
           const Kwargs &kwargs,
           const std::shared_ptr<Environment> &) -> Value {

            expectArgsRange(args, 1, 2, "sum");

            Value start = Value(Value::BigInt(0));   // по умолчанию — целый ноль
            bool startFromPositional = false;

            if (args.size() == 2) {
                start = args[1];
                startFromPositional = true;
            }

            for (const auto &[name, value] : kwargs) {          // start=... как kwarg
                if (name == "start") {
                    if (startFromPositional)
                        throw TypeErrorException("sum() got multiple values for argument 'start'");
                    start = value;
                } else {
                    throw TypeErrorException("'" + name + "' is an invalid keyword argument for sum()");
                }
            }

            if (start.isString())    throw TypeErrorException("sum() can't sum strings [use ''.join(seq) instead]");
            if (start.isBytes())     throw TypeErrorException("sum() can't sum bytes [use b''.join(seq) instead]");
            if (start.isByteArray()) throw TypeErrorException("sum() can't sum bytearray [use b''.join(seq) instead]");

            const auto it = args[0].getIterator();
            Value acc = start;

            while (true) {
                try {
                    acc = acc + it->next();          // семантика Value::operator+ — та же, что у интерпретатора
                } catch (const StopIterationException &) {
                    break;
                }
            }
            return acc;
        }
    ));

    env->set("all",
    makeBuiltin(
        "all",

        [](const std::vector<Value> &args,
           const Kwargs &kwargs,
           const std::shared_ptr<Environment> &) -> Value {

            expectArgs(args, 1, "all");
            expectNoKwargs(kwargs, "all");

            const auto it = args[0].getIterator();

            // короткое замыкание: первый ложный элемент — сразу False
            while (true) {
                Value item;
                try {
                    item = it->next();
                } catch (const StopIterationException &) {
                    break;
                }
                if (!item.toBool()) {
                    return Value(false);
                }
            }
            // пустой итерируемый или все элементы истинны
            return Value(true);
        }
    ));

    env->set("any",
    makeBuiltin(
        "any",

        [](const std::vector<Value> &args,
           const Kwargs &kwargs,
           const std::shared_ptr<Environment> &) -> Value {

            expectArgs(args, 1, "any");
            expectNoKwargs(kwargs, "any");

            const auto it = args[0].getIterator();

            while (true) {
                Value item;
                try {
                    item = it->next();
                } catch (const StopIterationException &) {
                    break;
                }
                if (item.toBool()) {
                    return Value(true);
                }
            }
            return Value(false);
        }
    ));

        env->set("min",
        makeBuiltin(
            "min",

            [](const std::vector<Value> &args,
               const Kwargs &kwargs,
               const std::shared_ptr<Environment> &env) -> Value {

                if (args.empty()) {
                    throw TypeErrorException(
                        "min expected at least 1 argument, got 0"
                    );
                }

                std::optional<Value> key;
                std::optional<Value> defaultValue;

                for (const auto &[name, value]: kwargs) {

                    if (name == "key") {
                        // key=None означает «без ключа»
                        if (!value.isNone()) {
                            key = value;
                        }
                    } else if (name == "default") {
                        defaultValue = value;
                    } else {
                        throw TypeErrorException(
                            "'" + name + "' is an invalid keyword argument for min()"
                        );
                    }
                }

                // две формы: min(iterable) и min(a, b, ...)
                const bool multiArg = args.size() > 1;

                if (multiArg && defaultValue.has_value()) {
                    throw TypeErrorException(
                        "Cannot specify a default for min() with multiple positional arguments"
                    );
                }

                Value best;
                Value bestKey;
                bool hasBest = false;

                auto consider = [&](const Value &elem) {

                    const Value elemKey =
                        key.has_value()
                            ? call(key.value(), {elem}, {}, env)
                            : elem;

                    // строгое < — при равных ключах остаётся ПЕРВЫЙ минимум
                    if (!hasBest || elemKey < bestKey) {
                        best = elem;
                        bestKey = elemKey;
                        hasBest = true;
                    }
                };

                if (multiArg) {
                    for (const auto &elem: args) {
                        consider(elem);
                    }
                } else {
                    const auto it = args[0].getIterator();

                    while (true) {
                        Value item;
                        try {
                            item = it->next();
                        } catch (const StopIterationException &) {
                            break;
                        }
                        consider(item);
                    }
                }

                if (!hasBest) {
                    if (defaultValue.has_value()) {
                        return defaultValue.value();
                    }
                    throw ValueErrorException("min() arg is an empty sequence");
                }

                return best;
            }
        ));

        env->set("max",
        makeBuiltin(
            "max",

            [](const std::vector<Value> &args,
               const Kwargs &kwargs,
               const std::shared_ptr<Environment> &env) -> Value {

                if (args.empty()) {
                    throw TypeErrorException(
                        "max expected at least 1 argument, got 0"
                    );
                }

                std::optional<Value> key;
                std::optional<Value> defaultValue;

                for (const auto &[name, value]: kwargs) {

                    if (name == "key") {
                        // key=None означает «без ключа»
                        if (!value.isNone()) {
                            key = value;
                        }
                    } else if (name == "default") {
                        defaultValue = value;
                    } else {
                        throw TypeErrorException(
                            "'" + name + "' is an invalid keyword argument for max()"
                        );
                    }
                }

                // две формы: max(iterable) и max(a, b, ...)
                const bool multiArg = args.size() > 1;

                if (multiArg && defaultValue.has_value()) {
                    throw TypeErrorException(
                        "Cannot specify a default for max() with multiple positional arguments"
                    );
                }

                Value best;
                Value bestKey;
                bool hasBest = false;

                auto consider = [&](const Value &elem) {

                    const Value elemKey =
                        key.has_value()
                            ? call(key.value(), {elem}, {}, env)
                            : elem;

                    // строгое > — при равных ключах остаётся ПЕРВЫЙ максимум
                    if (!hasBest || elemKey > bestKey) {
                        best = elem;
                        bestKey = elemKey;
                        hasBest = true;
                    }
                };

                if (multiArg) {
                    for (const auto &elem: args) {
                        consider(elem);
                    }
                } else {
                    const auto it = args[0].getIterator();

                    while (true) {
                        Value item;
                        try {
                            item = it->next();
                        } catch (const StopIterationException &) {
                            break;
                        }
                        consider(item);
                    }
                }

                if (!hasBest) {
                    if (defaultValue.has_value()) {
                        return defaultValue.value();
                    }
                    throw ValueErrorException("max() arg is an empty sequence");
                }

                return best;
            }
        ));

    env->set("sorted",
    makeBuiltin(
        "sorted",

        [](const std::vector<Value> &args,
           const Kwargs &kwargs,
           const std::shared_ptr<Environment> &env) -> Value {

            expectArgs(args, 1, "sorted");

            std::optional<Value> key;
            bool reverse = false;

            for (const auto &[name, value]: kwargs) {

                if (name == "key") {
                    // key=None означает «без ключа»
                    if (!value.isNone()) {
                        key = value;
                    }
                } else if (name == "reverse") {
                    reverse = value.toBool();
                } else {
                    throw TypeErrorException(
                        "'" + name + "' is an invalid keyword argument for sorted()"
                    );
                }
            }

            // материализуем произвольное итерируемое в НОВЫЙ список
            const auto it = args[0].getIterator();

            std::vector<Value> items;

            while (true) {
                try {
                    items.push_back(it->next());
                } catch (const StopIterationException &) {
                    break;
                }
            }

            const auto result = std::make_shared<ListValue>(items);

            // переиспользуем стабильную сортировку списка (key/reverse)
            result->sort(key, reverse, env);

            return Value(result);
        }
    ));

    env->set("map",
    makeBuiltin(
        "map",

        [](const std::vector<Value> &args,
           const Kwargs &kwargs,
           const std::shared_ptr<Environment> &env) -> Value {

            expectNoKwargs(kwargs, "map");

            if (args.size() < 2) {
                throw TypeErrorException(
                    "map() must have at least two arguments."
                );
            }

            const Value &func = args[0];

            // получаем итераторы всех входных последовательностей
            std::vector<Value::IteratorPtr> sources;
            sources.reserve(args.size() - 1);

            for (std::size_t i = 1; i < args.size(); ++i) {
                sources.push_back(args[i].getIterator());
            }

            return Value(std::make_shared<MapIterator>(
                func, std::move(sources), env
            ));
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
