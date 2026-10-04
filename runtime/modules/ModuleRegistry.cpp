//
// Created by semyo on 03.10.2026.
//

#include "ModuleRegistry.h"
#include "BuiltinFunction.h"
#include "CallRuntime.h"
#include "DictValue.h"
#include "IteratorValue.h"
#include "ModuleValue.h"
#include "PartialValue.h"
#include "TupleValue.h"
#include "../ArgValidation.h"
#include "../../exception/StopIterationException.h"
#include "../../exception/TypeErrorException.h"
#include "../../exception/ValueErrorException.h"

static Value functoolsReduce(const std::vector<Value> &args, const Kwargs &kwargs,
                             const std::shared_ptr<Environment> &env) {
    expectNoKwargs(kwargs, "reduce");
    expectArgsRange(args, 2, 3, "reduce");

    const Value &func = args[0];
    const auto it = args[1].getIterator();   // не итерируемое -> TypeError

    Value acc;
    bool hasAcc = false;
    if (args.size() == 3) { acc = args[2]; hasAcc = true; }

    while (true) {
        Value item;
        try { item = it->next(); }
        catch (const StopIterationException &) { break; }
        if (!hasAcc) { acc = item; hasAcc = true; }
        else { acc = call(func, { acc, item }, {}, env); }
    }

    if (!hasAcc) {
        throw TypeErrorException("reduce() of empty iterable with no initial value");
    }
    return acc;
}

// functools.partial(func, *args, **keywords)
static Value functoolsPartial(const std::vector<Value> &args,
                              const Kwargs &kwargs,
                              const std::shared_ptr<Environment> &) {

    if (args.empty()) {
        throw TypeErrorException("partial expected at least 1 argument, got 0");
    }

    const Value &func = args[0];

    if (!func.isCallable()) {
        throw TypeErrorException("the first argument must be callable");
    }

    std::vector<Value> bound(args.begin() + 1, args.end());

    return Value(std::make_shared<PartialValue>(func, bound, kwargs));
}

// functools.total_ordering(cls) — дописывает недостающие операторы сравнения
static Value functoolsTotalOrdering(const std::vector<Value> &args,
                                    const Kwargs &kwargs,
                                    const std::shared_ptr<Environment> &) {

    expectArgs(args, 1, "total_ordering");
    expectNoKwargs(kwargs, "total_ordering");

    if (!args[0].isClass()) {
        throw TypeErrorException("total_ordering() argument must be a class");
    }

    const auto cls = args[0].asClass();

    const bool hasLt = cls->attributes.contains("__lt__");
    const bool hasGt = cls->attributes.contains("__gt__");
    const bool hasLe = cls->attributes.contains("__le__");
    const bool hasGe = cls->attributes.contains("__ge__");

    if (!hasLt && !hasGt && !hasLe && !hasGe) {
        throw ValueErrorException(
            "must define at least one ordering operation: < > <= >="
        );
    }

    // синтез метода-билтина: получает (self, other) через BoundMethod
    auto method = [](const QString &name,
                     std::function<bool(const Value &, const Value &)> fn) {
        return Value(std::make_shared<BuiltinFunction>(
            name,
            [fn](const std::vector<Value> &a,
                 const Kwargs &,
                 const std::shared_ptr<Environment> &) -> Value {
                return Value(fn(a[0], a[1]));
            }));
    };

    // недостающие операторы выражаются через оператор-основу (без рекурсии)
    if (hasLt) {
        if (!hasGt) cls->attributes["__gt__"] = method("__gt__", [](const Value &a, const Value &b) { return b < a; });
        if (!hasLe) cls->attributes["__le__"] = method("__le__", [](const Value &a, const Value &b) { return !(b < a); });
        if (!hasGe) cls->attributes["__ge__"] = method("__ge__", [](const Value &a, const Value &b) { return !(a < b); });
    } else if (hasGt) {
        if (!hasLt) cls->attributes["__lt__"] = method("__lt__", [](const Value &a, const Value &b) { return b > a; });
        if (!hasGe) cls->attributes["__ge__"] = method("__ge__", [](const Value &a, const Value &b) { return !(b > a); });
        if (!hasLe) cls->attributes["__le__"] = method("__le__", [](const Value &a, const Value &b) { return !(a > b); });
    } else if (hasLe) {
        if (!hasGe) cls->attributes["__ge__"] = method("__ge__", [](const Value &a, const Value &b) { return b <= a; });
        if (!hasLt) cls->attributes["__lt__"] = method("__lt__", [](const Value &a, const Value &b) { return !(b <= a); });
        if (!hasGt) cls->attributes["__gt__"] = method("__gt__", [](const Value &a, const Value &b) { return !(a <= b); });
    } else {
        if (!hasLe) cls->attributes["__le__"] = method("__le__", [](const Value &a, const Value &b) { return b >= a; });
        if (!hasGt) cls->attributes["__gt__"] = method("__gt__", [](const Value &a, const Value &b) { return !(b >= a); });
        if (!hasLt) cls->attributes["__lt__"] = method("__lt__", [](const Value &a, const Value &b) { return !(a >= b); });
    }

    return args[0];   // декоратор возвращает тот же класс
}

// functools.cmp_to_key(cmp) — превращает cmp-функцию в key-функцию
static Value functoolsCmpToKey(const std::vector<Value> &args,
                               const Kwargs &kwargs,
                               const std::shared_ptr<Environment> &) {

    expectArgs(args, 1, "cmp_to_key");
    expectNoKwargs(kwargs, "cmp_to_key");

    const Value cmp = args[0];

    // класс-обёртка, специализированный под данную cmp-функцию
    auto K = std::make_shared<ClassValue>("functools.KeyWrapper");

    if (Runtime::objectClass) {
        K->bases.push_back(Runtime::objectClass);
    }

    // __init__(self, obj): сохраняем сравниваемое значение
    K->attributes["__init__"] = Value(std::make_shared<BuiltinFunction>(
        "__init__",
        [](const std::vector<Value> &a,
           const Kwargs &,
           const std::shared_ptr<Environment> &) -> Value {
            a[0].asInstance()->fields["obj"] = a[1];
            return {};
        }));

    // фабрика метода сравнения: pred(cmp(self.obj, other.obj))
    auto cmpMethod = [cmp](std::function<bool(const Value &)> pred) {
        return Value(std::make_shared<BuiltinFunction>(
            "cmp_op",
            [cmp, pred](const std::vector<Value> &a,
                        const Kwargs &,
                        const std::shared_ptr<Environment> &) -> Value {
                const Value selfObj  = a[0].asInstance()->fields["obj"];
                const Value otherObj = a[1].asInstance()->fields["obj"];
                const Value r = call(cmp, { selfObj, otherObj }, {}, nullptr);
                return Value(pred(r));
            }));
    };

    K->attributes["__lt__"] = cmpMethod([](const Value &r) { return r <  Value(Value::BigInt(0)); });
    K->attributes["__gt__"] = cmpMethod([](const Value &r) { return r >  Value(Value::BigInt(0)); });
    K->attributes["__le__"] = cmpMethod([](const Value &r) { return r <= Value(Value::BigInt(0)); });
    K->attributes["__ge__"] = cmpMethod([](const Value &r) { return r >= Value(Value::BigInt(0)); });
    K->attributes["__eq__"] = cmpMethod([](const Value &r) { return r == Value(Value::BigInt(0)); });

    return Value(K);
}

// functools.cache(func) — мемоизация без вытеснения (== lru_cache(maxsize=None))
static Value functoolsCache(const std::vector<Value> &args,
                            const Kwargs &kwargs,
                            const std::shared_ptr<Environment> &) {

    expectArgs(args, 1, "cache");
    expectNoKwargs(kwargs, "cache");

    const Value func = args[0];

    if (!func.isCallable()) {
        throw TypeErrorException("the first argument must be callable");
    }

    // общий на все вызовы обёртки словарь-кэш
    auto cacheDict = std::make_shared<DictValue>();

    return Value(std::make_shared<BuiltinFunction>(
        "cache_wrapper",
        [func, cacheDict](const std::vector<Value> &callArgs,
                          const Kwargs &callKwargs,
                          const std::shared_ptr<Environment> &e) -> Value {

            // ключ — кортеж позиционных аргументов (+ отсортированные kwargs)
            std::vector<Value> keyItems = callArgs;

            if (!callKwargs.empty()) {
                std::vector<std::pair<QString, Value>> kw(
                    callKwargs.begin(), callKwargs.end());

                std::sort(kw.begin(), kw.end(),
                    [](const auto &x, const auto &y) { return x.first < y.first; });

                keyItems.push_back(Value(QString("__kwargs__")));
                for (const auto &[k, v] : kw) {
                    keyItems.push_back(Value(k));
                    keyItems.push_back(v);
                }
            }

            const auto key = Value(std::make_shared<TupleValue>(keyItems));

            if (cacheDict->contains(key)) {
                return cacheDict->getItem(key);
            }

            const Value result = call(func, callArgs, callKwargs, e);
            cacheDict->setItem(key, result);
            return result;
        }));
}

static Value::ModulePtr makeFunctoolsModule() {

    auto mod = std::make_shared<ModuleValue>("functools");

    mod->members["__name__"] = Value(QString("functools"));

    mod->members["reduce"] =
        Value(std::make_shared<BuiltinFunction>("reduce", functoolsReduce));

    mod->members["partial"] =
        Value(std::make_shared<BuiltinFunction>("partial", functoolsPartial));

    mod->members["total_ordering"] =
        Value(std::make_shared<BuiltinFunction>("total_ordering", functoolsTotalOrdering));

    mod->members["cmp_to_key"] =
        Value(std::make_shared<BuiltinFunction>("cmp_to_key", functoolsCmpToKey));

    mod->members["cache"] =
        Value(std::make_shared<BuiltinFunction>("cache", functoolsCache));

    return mod;
}

Value::ModulePtr importBuiltinModule(const QString &name) {
    if (name == "functools") {
        return makeFunctoolsModule();
    }
    return nullptr;
}