//
// Created by semyo on 03.10.2026.
//

#include "ModuleRegistry.h"
#include "BuiltinFunction.h"
#include "CallRuntime.h"
#include "IteratorValue.h"
#include "ModuleValue.h"
#include "PartialValue.h"
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

static Value::ModulePtr makeFunctoolsModule() {

    auto mod = std::make_shared<ModuleValue>("functools");

    mod->members["__name__"] = Value(QString("functools"));

    mod->members["reduce"] =
        Value(std::make_shared<BuiltinFunction>("reduce", functoolsReduce));

    mod->members["partial"] =
        Value(std::make_shared<BuiltinFunction>("partial", functoolsPartial));

    mod->members["total_ordering"] =
        Value(std::make_shared<BuiltinFunction>("total_ordering", functoolsTotalOrdering));

    return mod;
}

Value::ModulePtr importBuiltinModule(const QString &name) {
    if (name == "functools") {
        return makeFunctoolsModule();
    }
    return nullptr;
}