#include "BoundMethod.h"
#include "ByteArrayValue.h"
#include "BytesValue.h"
#include "CallRuntime.h"
#include "ClassMethodValue.h"
#include "ClassUtils.h"
#include "DictValue.h"
#include "EnumerateIterator.h"
#include "Environment.h"
#include "FilterIterator.h"
#include "FrozenSetValue.h"
#include "FunctionValue.h"
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
#include "ZipIterator.h"
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

// Банковское округление вещественного до целого: половина — к чётному.
static Value::BigInt roundHalfEvenToInt(const Value::BigFloat &y) {

    const Value::BigInt trunc = y.convert_to<Value::BigInt>();  // усечение к нулю
    const Value::BigFloat truncF = trunc.convert_to<Value::BigFloat>();

    // floor и дробная часть от него в [0, 1)
    Value::BigInt floor;
    Value::BigFloat frac;

    if (y >= truncF) {
        floor = trunc;
        frac = y - truncF;
    } else {
        // отрицательное с ненулевой дробной частью: floor = trunc - 1
        floor = trunc - 1;
        frac = y - floor.convert_to<Value::BigFloat>();
    }

    const Value::BigFloat half("0.5");

    if (frac < half) {
        return floor;
    }
    if (frac > half) {
        return floor + 1;
    }

    // ровно половина — округляем к чётному
    return (floor % 2 == 0) ? floor : floor + 1;
}

// Банковское округление целого до кратного scale (scale > 0): половина — к чётному.
static Value::BigInt roundIntHalfEven(const Value::BigInt &x, const Value::BigInt &scale) {

    const Value::BigInt q = x / scale;           // усечение к нулю
    const Value::BigInt r = x - q * scale;       // остаток со знаком x
    const Value::BigInt twice = 2 * (r < 0 ? -r : r);

    const Value::BigInt step = (x >= 0) ? 1 : -1;

    Value::BigInt result = q;

    if (twice > scale) {
        result = q + step;
    } else if (twice == scale && q % 2 != 0) {
        result = q + step;   // ровно половина — к чётному
    }

    return result * scale;
}

// Степень десятки (10^n) как вещественное; n может быть отрицательным.
static Value::BigFloat pow10(long long n) {

    Value::BigFloat r = 1;

    if (n >= 0) {
        for (long long i = 0; i < n; ++i) r *= 10;
    } else {
        for (long long i = 0; i < -n; ++i) r /= 10;
    }

    return r;
}

// Python-остаток (floor-mod): знак результата совпадает со знаком m.
static Value::BigInt floorMod(const Value::BigInt &a, const Value::BigInt &m) {
    Value::BigInt r = a % m;
    if (r != 0 && ((r < 0) != (m < 0))) {
        r += m;
    }
    return r;
}

// Быстрое модульное возведение в степень (exp >= 0), результат в терминах floor-mod.
static Value::BigInt powMod(Value::BigInt base, Value::BigInt exp, const Value::BigInt &mod) {
    Value::BigInt result = floorMod(1, mod);   // для mod == 1 это 0
    base = floorMod(base, mod);

    while (exp > 0) {
        if (exp % 2 != 0) {
            result = floorMod(result * base, mod);
        }
        base = floorMod(base * base, mod);
        exp /= 2;
    }
    return result;
}

// Расширенный алгоритм Евклида: возвращает НОД(a, b) и x, y такие, что a*x + b*y = НОД.
static Value::BigInt extGcd(const Value::BigInt &a, const Value::BigInt &b,
                            Value::BigInt &x, Value::BigInt &y) {
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }

    Value::BigInt x1, y1;
    const Value::BigInt g = extGcd(b, a % b, x1, y1);

    x = y1;
    y = x1 - (a / b) * y1;
    return g;
}

// Модульная инверсия a по модулю mod; ошибка, если a необратимо.
static Value::BigInt modInverse(const Value::BigInt &a, const Value::BigInt &mod) {
    const Value::BigInt m = (mod < 0) ? -mod : mod;

    Value::BigInt x, y;
    const Value::BigInt g = extGcd(floorMod(a, m), m, x, y);

    if (g != 1) {
        throw ValueErrorException(
            "base is not invertible for the given modulus"
        );
    }

    return floorMod(x, mod);
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

        env->set("pow",
        makeBuiltin(
            "pow",

            [](const std::vector<Value> &args,
               const Kwargs &kwargs,
               const std::shared_ptr<Environment> &) -> Value {

                expectArgsRange(args, 2, 3, "pow");

                // mod из третьего позиционного аргумента или именованного mod=
                Value modVal;
                bool modProvided = false;

                if (args.size() == 3) {
                    modVal = args[2];
                    modProvided = true;
                }

                for (const auto &[name, value]: kwargs) {
                    if (name == "mod") {
                        if (modProvided) {
                            throw TypeErrorException(
                                "pow() got multiple values for argument 'mod'"
                            );
                        }
                        modVal = value;
                        modProvided = true;
                    } else {
                        throw TypeErrorException(
                            "'" + name + "' is an invalid keyword argument for pow()"
                        );
                    }
                }

                const Value &base = args[0];
                const Value &exp = args[1];

                // трёхаргументная форма: (base ** exp) % mod с быстрым модульным возведением
                if (modProvided && !modVal.isNone()) {

                    const bool allInts =
                        (base.isBigInt() || base.isBool()) &&
                        (exp.isBigInt()  || exp.isBool())  &&
                        (modVal.isBigInt() || modVal.isBool());

                    if (!allInts) {
                        throw TypeErrorException(
                            "pow() 3rd argument not allowed unless all arguments are integers"
                        );
                    }

                    const Value::BigInt m = modVal.toBigInt();

                    if (m == 0) {
                        throw ValueErrorException("pow() 3rd argument cannot be 0");
                    }

                    const Value::BigInt b = base.toBigInt();
                    const Value::BigInt e = exp.toBigInt();

                    if (e >= 0) {
                        return Value(powMod(b, e, m));
                    }

                    // отрицательная степень — через модульную инверсию основания
                    return Value(powMod(modInverse(b, m), -e, m));
                }

                // двухаргументная форма — обычное возведение в степень
                return base.power(exp);
            }
        ));

    env->set("__list_call__",
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

    env->set("__tuple_call__",
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

    env->set("__set_call__",
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

    env->set("__dict_call__",
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

    env->set("__int_call__",
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

    env->set("__bool_call__",
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

    env->set("__float_call__",
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

    env->set("filter",
    makeBuiltin(
        "filter",

        [](const std::vector<Value> &args,
           const Kwargs &kwargs,
           const std::shared_ptr<Environment> &env) -> Value {

            expectNoKwargs(kwargs, "filter");
            expectArgs(args, 2, "filter");

            const Value &predicate = args[0];

            // получаем итератор входной последовательности
            const auto source = args[1].getIterator();

            return Value(std::make_shared<FilterIterator>(
                predicate, source, env
            ));
        }
    ));

    env->set("zip",
    makeBuiltin(
        "zip",

        [](const std::vector<Value> &args,
           const Kwargs &kwargs,
           const std::shared_ptr<Environment> &) -> Value {

            bool strict = false;

            for (const auto &[name, value]: kwargs) {
                if (name == "strict") {
                    strict = value.toBool();
                } else {
                    throw TypeErrorException(
                        "'" + name + "' is an invalid keyword argument for zip()"
                    );
                }
            }

            // получаем итераторы всех входных последовательностей
            std::vector<Value::IteratorPtr> sources;
            sources.reserve(args.size());

            for (const auto &arg: args) {
                sources.push_back(arg.getIterator());
            }

            return Value(std::make_shared<ZipIterator>(
                std::move(sources), strict
            ));
        }
    ));

    env->set("enumerate",
    makeBuiltin(
        "enumerate",

        [](const std::vector<Value> &args,
           const Kwargs &kwargs,
           const std::shared_ptr<Environment> &) -> Value {

            expectArgsRange(args, 1, 2, "enumerate");

            // start по умолчанию — целочисленный ноль
            Value startVal = Value(Value::BigInt(0));
            bool startPositional = false;

            if (args.size() == 2) {
                startVal = args[1];
                startPositional = true;
            }

            // start можно передать и как именованный аргумент
            for (const auto &[name, value]: kwargs) {

                if (name == "start") {

                    if (startPositional) {
                        throw TypeErrorException(
                            "enumerate() got multiple values for argument 'start'"
                        );
                    }

                    startVal = value;
                } else {
                    throw TypeErrorException(
                        "'" + name + "' is an invalid keyword argument for enumerate()"
                    );
                }
            }

            // start обязан быть целым (bool — подтип int в Python)
            if (!startVal.isBigInt() && !startVal.isBool()) {
                throw TypeErrorException(
                    "'" + startVal.getTypeName() +
                    "' object cannot be interpreted as an integer"
                );
            }

            const auto source = args[0].getIterator();

            const Value::BigInt start = startVal.isBool()
                ? Value::BigInt(startVal.toBool() ? 1 : 0)
                : startVal.toBigInt();

            return Value(std::make_shared<EnumerateIterator>(
                source, start
            ));
        }
    ));

    env->set("callable",
    makeBuiltin(
        "callable",

        [](const std::vector<Value> &args,
           const Kwargs &kwargs,
           const std::shared_ptr<Environment> &) -> Value {

            expectArgs(args, 1, "callable");
            expectNoKwargs(kwargs, "callable");

            return Value(args[0].isCallable());
        }
    ));

    env->set("abs",
    makeBuiltin(
        "abs",

        [](const std::vector<Value> &args,
           const Kwargs &kwargs,
           const std::shared_ptr<Environment> &) -> Value {

            expectArgs(args, 1, "abs");
            expectNoKwargs(kwargs, "abs");

            const Value &x = args[0];

            // bool — подтип int, поэтому abs(True) == 1 (int)
            if (x.isBigInt() || x.isBool()) {
                const Value::BigInt v = x.toBigInt();
                return Value(v < 0 ? Value::BigInt(-v) : v);
            }

            if (x.isBigFloat()) {
                const Value::BigFloat v = x.toBigFloat();
                return Value(v < 0 ? Value::BigFloat(-v) : v);
            }

            throw TypeErrorException(
                "bad operand type for abs(): '" + x.getTypeName() + "'"
            );
        }
    ));

    env->set("round",
    makeBuiltin(
        "round",

        [](const std::vector<Value> &args,
           const Kwargs &kwargs,
           const std::shared_ptr<Environment> &) -> Value {

            expectArgsRange(args, 1, 2, "round");

            const Value &x = args[0];

            // ndigits: отсутствует / None (→ результат int) либо целое
            Value ndigitsVal;
            bool ndigitsProvided = false;

            if (args.size() == 2) {
                ndigitsVal = args[1];
                ndigitsProvided = true;
            }

            for (const auto &[name, value]: kwargs) {
                if (name == "ndigits") {
                    if (ndigitsProvided) {
                        throw TypeErrorException(
                            "round() got multiple values for argument 'ndigits'"
                        );
                    }
                    ndigitsVal = value;
                    ndigitsProvided = true;
                } else {
                    throw TypeErrorException(
                        "'" + name + "' is an invalid keyword argument for round()"
                    );
                }
            }

            bool hasNdigits = false;
            Value::BigInt ndigits = 0;

            if (ndigitsProvided && !ndigitsVal.isNone()) {
                if (!ndigitsVal.isBigInt() && !ndigitsVal.isBool()) {
                    throw TypeErrorException(
                        "'" + ndigitsVal.getTypeName() +
                        "' object cannot be interpreted as an integer"
                    );
                }
                ndigits = ndigitsVal.toBigInt();
                hasNdigits = true;
            }

            // округлять умеем только числа
            if (!x.isBigInt() && !x.isBool() && !x.isBigFloat()) {
                throw TypeErrorException(
                    "type " + x.getTypeName() + " doesn't define __round__ method"
                );
            }

            // без ndigits — результат int
            if (!hasNdigits) {
                if (x.isBigInt() || x.isBool()) {
                    return Value(x.toBigInt());
                }
                return Value(roundHalfEvenToInt(x.toBigFloat()));
            }

            // с ndigits: int остаётся int
            if (x.isBigInt() || x.isBool()) {
                const Value::BigInt xi = x.toBigInt();

                if (ndigits >= 0) {
                    return Value(xi);
                }

                // отрицательные ndigits — округление к 10^(-ndigits)
                Value::BigInt scale = 1;
                for (Value::BigInt i = 0; i < -ndigits; ++i) {
                    scale *= 10;
                }
                return Value(roundIntHalfEven(xi, scale));
            }

            // float с ndigits — результат float
            const Value::BigFloat xf = x.toBigFloat();
            const Value::BigFloat scale = pow10(ndigits.convert_to<long long>());
            const Value::BigInt rounded = roundHalfEvenToInt(xf * scale);

            return Value(rounded.convert_to<Value::BigFloat>() / scale);
        }
    ));

    env->set("divmod",
    makeBuiltin(
        "divmod",

        [](const std::vector<Value> &args,
           const Kwargs &kwargs,
           const std::shared_ptr<Environment> &) -> Value {

            expectArgs(args, 2, "divmod");
            expectNoKwargs(kwargs, "divmod");

            const Value &a = args[0];
            const Value &b = args[1];

            // floor-деление; сам бросает ZeroDivisionError / TypeError
            const Value div = a.intDivide(b);

            // остаток, согласованный с floor-делением: a == div * b + mod
            const Value mod = a - div * b;

            return Value(std::make_shared<TupleValue>(
                std::vector<Value>{ div, mod }
            ));
        }
    ));

    env->set("type",
    makeBuiltin(
        "type",

        [](const std::vector<Value> &args,
           const Kwargs &kwargs,
           const std::shared_ptr<Environment> &) -> Value {

            expectNoKwargs(kwargs, "type");

            // 1-аргументная форма type(x) появится в следующем коммите
            if (args.size() != 3) {
                throw TypeErrorException("type() takes 1 or 3 arguments");
            }

            const Value &nameV = args[0];
            const Value &basesV = args[1];
            const Value &nsV = args[2];

            if (!nameV.isString()) {
                throw TypeErrorException("type() argument 1 must be str");
            }

            if (!basesV.isTuple()) {
                throw TypeErrorException("type() argument 2 must be a tuple of classes");
            }

            if (!nsV.isDict()) {
                throw TypeErrorException("type() argument 3 must be a dict");
            }

            const QString name = nameV.asString("type()")->getValue();

            std::vector<Value::ClassPtr> bases;

            for (const Value &b : basesV.asTuple("type()")->items) {

                if (!b.isClass()) {
                    throw TypeErrorException("bases must be classes");
                }

                bases.push_back(b.asClass());
            }

            if (bases.empty()) {
                bases.push_back(Runtime::objectClass);
            }

            const auto cls = std::make_shared<ClassValue>(name);
            cls->bases = bases;

            const auto ns = nsV.asDict("type()");

            // переносим пространство имён в атрибуты, сохраняя порядок
            for (const Value &keyV : ns->getOrder()) {

                if (!keyV.isString()) {
                    continue;
                }

                const QString key = keyV.asString("type()")->getValue();
                Value val = ns->getItem(keyV);

                // методам нужен владелец-класс (для привязки self)
                if (val.isFunction()) {
                    val.asFunction()->ownerClass = cls;
                } else if (val.isStaticMethod()) {
                    val.asStaticMethod()->func->ownerClass = cls;
                } else if (val.isClassMethod()) {
                    val.asClassMethod()->func->ownerClass = cls;
                }

                cls->attributes.insert(key, val);
            }

            return Value(cls);
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
