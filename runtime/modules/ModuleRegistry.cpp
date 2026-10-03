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

static Value::ModulePtr makeFunctoolsModule() {

    auto mod = std::make_shared<ModuleValue>("functools");

    mod->members["__name__"] = Value(QString("functools"));

    mod->members["reduce"] =
        Value(std::make_shared<BuiltinFunction>("reduce", functoolsReduce));

    mod->members["partial"] =
        Value(std::make_shared<BuiltinFunction>("partial", functoolsPartial));

    return mod;
}

Value::ModulePtr importBuiltinModule(const QString &name) {
    if (name == "functools") {
        return makeFunctoolsModule();
    }
    return nullptr;
}