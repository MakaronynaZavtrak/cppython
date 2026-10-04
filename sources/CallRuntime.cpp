#include "CallRuntime.h"

#include "BoundMethod.h"
#include "ByteArrayValue.h"
#include "BytesValue.h"
#include "ClassMethodValue.h"
#include "Environment.h"
#include "FunctionValue.h"
#include "Parser.h"
#include "StaticMethodValue.h"
#include "StrValue.h"
#include "Value.h"

#include <unordered_set>

#include "ClassUtils.h"
#include "DictValue.h"
#include "GeneratorValue.h"
#include "Interpreter.h"
#include "IteratorValue.h"
#include "PartialValue.h"
#include "RangeValue.h"
#include "TupleValue.h"
#include "../exception/AttributeErrorException.h"
#include "../exception/LookupErorException.h"
#include "../exception/ReturnException.h"
#include "../exception/TypeErrorException.h"
#include "../exception/ValueErrorException.h"
#include "../runtime/ArgValidation.h"

//
// Created by semyo on 03.05.2026.
//

void bindParams(const std::shared_ptr<Environment>& local,
                const Value::FunctionPtr& func,
                const std::vector<Value>& args,
                const Kwargs& kwargs) {

    std::unordered_set<QString> assigned;

    // индекс varargs-параметра, если он есть
    int varArgsIdx = -1;
    int kwArgsIdx = -1;

    // позиционно можно передать только то, что идёт ДО *args / голой * / **kwargs
    size_t positionalLimit = func->params.size();

    for (size_t i = 0; i < func->params.size(); ++i) {
        const auto& p = func->params[i];
        if (p.isVarArgs || p.isKwArgs || p.isKeywordOnly) {
            positionalLimit = i;
            break;
        }
    }

    for (size_t i = 0; i < func->params.size(); ++i) {
        if (func->params[i].isVarArgs) varArgsIdx = static_cast<int>(i);
        if (func->params[i].isKwArgs)  kwArgsIdx = static_cast<int>(i);
    }

    if (varArgsIdx >= 0) {
        positionalLimit = static_cast<size_t>(varArgsIdx);
    } else if (kwArgsIdx >= 0) {
        positionalLimit = static_cast<size_t>(kwArgsIdx);
    }

    // обычные позиционные
    for (size_t i = 0; i < args.size() && i < positionalLimit; ++i) {
        const QString& paramName = func->params[i].name;
        local->set(paramName, args[i]);
        assigned.insert(paramName);
    }

    // лишние позиционные -> *args
    if (args.size() > positionalLimit) {

        if (varArgsIdx < 0) {
            throw TypeErrorException(
                func->name + "() takes " + QString::number(positionalLimit) +
                " positional arguments but " + QString::number(args.size()) + " were given"
            );
        }

        std::vector extra(args.begin() + positionalLimit, args.end());

        local->set(func->params[varArgsIdx].name,
                   Value(std::make_shared<TupleValue>(extra)));
        assigned.insert(func->params[varArgsIdx].name);
    }
    else if (varArgsIdx >= 0) {
        // *args есть, но лишних аргументов нет — пустой кортеж
        local->set(func->params[varArgsIdx].name,
                   Value(std::make_shared<TupleValue>(std::vector<Value>{})));
        assigned.insert(func->params[varArgsIdx].name);
    }

    // именованные
    const auto kwDict = std::make_shared<DictValue>();
    QStringList posOnlyViolations;

    for (const auto& [name, value] : kwargs) {

        bool found = false;
        bool isPosOnlyName = false;

        for (const auto& param : func->params) {

            if (param.isVarArgs || param.isKwArgs) continue;

            if (param.name == name) {

                if (param.isPositionalOnly) {
                    isPosOnlyName = true;
                    break;
                }

                if (assigned.count(name)) {
                    throw TypeErrorException(
                        func->name + "() got multiple values for argument '" + name + "'"
                    );
                }

                local->set(name, value);
                assigned.insert(name);
                found = true;
                break;
            }
        }

        if (isPosOnlyName) {

            // при наличии **kwargs имя уходит туда — это законно в Python
            if (kwArgsIdx >= 0) {
                kwDict->setItem(Value(name), value);
                continue;
            }

            posOnlyViolations.append(name);
            continue;
        }

        if (!found) {

            if (kwArgsIdx < 0) {
                throw TypeErrorException(
                    func->name + "() got an unexpected keyword argument '" + name + "'"
                );
            }

            kwDict->setItem(Value(name), value);
        }
    }

    if (!posOnlyViolations.isEmpty()) {
        throw TypeErrorException(
            func->name + "() got some positional-only arguments passed as keyword arguments: '"
            + posOnlyViolations.join(", ") + "'"
        );
    }

    if (kwArgsIdx >= 0) {
        local->set(func->params[kwArgsIdx].name, Value(kwDict));
        assigned.insert(func->params[kwArgsIdx].name);
    }

    // недостающие — дефолты
    for (size_t i = 0; i < func->params.size(); ++i) {

        const auto& param = func->params[i];

        if (assigned.count(param.name)) continue;

        if (i < func->defaults.size() && func->defaults[i].has_value()) {
            local->set(param.name, func->defaults[i].value());
            continue;
        }

        if (param.isKeywordOnly) {
            throw TypeErrorException(
                func->name + "() missing 1 required keyword-only argument: '" + param.name + "'"
            );
        }

        throw TypeErrorException(
            func->name + "() missing required positional argument: '" + param.name + "'"
        );
    }
}

Value call(const Value& callee,
           const std::vector<Value>& args,
           const Kwargs& kwargs,
           const std::shared_ptr<Environment>& env) {

    if (callee.isBuiltinFunction()) {

        return callee.asBuiltinFunction()->func(args, kwargs, env);
    }

    if (const auto f = std::get_if<Value::FunctionPtr>(&callee.data)) {

        if ((*f)->isGenerator) {

            const auto local = std::make_shared<Environment>((*f)->closure);
            bindParams(local, *f, args, kwargs);

            auto gen = std::make_shared<GeneratorValue>();
            gen->func = *f;
            gen->env = local;

            return Value(gen);
        }

        return callFunction(*f, args, kwargs, nullptr);
    }

    if (const auto c = std::get_if<Value::ClassPtr>(&callee.data)) {
        return constructClass(*c, args, kwargs, env);
    }

    if (const auto b = std::get_if<Value::BoundMethodPtr>(&callee.data)) {
        return callBoundMethod(*b, args, kwargs);
    }

    if (const auto sm = std::get_if<Value::StaticMethodPtr>(&callee.data)) {
        return call(Value((*sm)->func), args, kwargs, env);
    }

    if (const auto cm = std::get_if<Value::ClassMethodPtr>(&callee.data)) {
        return call(Value((*cm)->func), args, kwargs, env);
    }

    if (const auto p = std::get_if<Value::PartialPtr>(&callee.data)) {

        // связанные аргументы идут в начало, затем аргументы вызова
        std::vector<Value> mergedArgs = (*p)->args;
        mergedArgs.insert(mergedArgs.end(), args.begin(), args.end());

        // связанные kwargs, поверх — kwargs вызова (они переопределяют)
        Kwargs mergedKwargs = (*p)->keywords;

        for (const auto &[key, value] : kwargs) {
            bool replaced = false;
            for (auto &[mKey, mValue] : mergedKwargs) {
                if (mKey == key) { mValue = value; replaced = true; break; }
            }
            if (!replaced) {
                mergedKwargs.emplace_back(key, value);
            }
        }

        return call((*p)->func, mergedArgs, mergedKwargs, env);
    }

    throw TypeErrorException("Object is not callable");
}

Value callFunction(const Value::FunctionPtr& func,
                   const std::vector<Value>& args,
                   const Kwargs& kwargs,
                   const std::shared_ptr<Environment>& envOverride = nullptr) {

    const auto local = std::make_shared<Environment>(func->closure);

    if (envOverride) {
        for (auto it = envOverride->variables.cbegin();
            it != envOverride->variables.cend(); ++it) {
            local->set(it.key(), it.value());
        }
    }

    bindParams(local, func, args, kwargs);

    const int srcId = func->body.empty() ? 0 : func->body[0]->sourceId;
    CallStackGuard guard(func->name, srcId);

    try {
        for (const auto& stmt : func->body) {
            Interpreter::executeNode(stmt, local);
        }
        return Value();
    }
    catch (ReturnException& e) {
        return e.getValue();
    }
}

bool supportsIter(const Value& obj) {

    try {

        getAttrValue(obj, "__iter__");

        return true;

    } catch (...) {

        return false;
    }
}

// __new__ встроенных типов: конструирование выполняется через штатный протокол,
// а constructClass диспетчеризует к ним обобщённо (без хардкода cls == ...).

static Value strNew(const std::vector<Value>& args, const Kwargs&,
                    const std::shared_ptr<Environment>&) {
    expectArgsRange(args, 0, 1, "str");
    if (args.empty()) {
        return Value("");
    }
    const Value& obj = args[0];
    try {
        Value strMethod = getAttrValue(obj, "__str__");
        Value result = call(strMethod, {}, {}, nullptr);
        if (!result.isString()) {
            throw TypeErrorException("__str__ returned non-string");
        }
        return result;
    } catch (const AttributeErrorException&) {
        return Value(obj.toString());
    }
}

static Value bytesNew(const std::vector<Value>& args, const Kwargs& kwargs,
                      const std::shared_ptr<Environment>&) {
    return Value(std::make_shared<BytesValue>(constructBytesData(args, kwargs)));
}

static Value bytearrayNew(const std::vector<Value>& args, const Kwargs& kwargs,
                          const std::shared_ptr<Environment>&) {
    return Value(std::make_shared<ByteArrayValue>(constructBytesData(args, kwargs)));
}

static Value rangeNew(const std::vector<Value>& args, const Kwargs&,
                      const std::shared_ptr<Environment>&) {
    for (const auto& a : args) {
        if (!a.isBigInt() && !a.isBool()) {
            throw TypeErrorException(
                "'" + a.repr() + "' object cannot be interpreted as an integer");
        }
    }
    Value::BigInt start = 0, stop, step = 1;
    if (args.size() == 1) {
        stop = args[0].toBigInt();
    } else if (args.size() == 2) {
        start = args[0].toBigInt();
        stop = args[1].toBigInt();
    } else if (args.size() == 3) {
        start = args[0].toBigInt();
        stop = args[1].toBigInt();
        step = args[2].toBigInt();
    } else {
        throw TypeErrorException("range expected at most 3 arguments, got " + QString::number(args.size()));
    }
    return Value(std::make_shared<RangeValue>(start, stop, step));
}

// Привязывает __new__ к class-объектам встроенных типов (вызывается после их создания).
void attachBuiltinNewMethods() {
    Runtime::strClass->attributes["__new__"]       = Value(std::make_shared<BuiltinFunction>("__new__", strNew));
    Runtime::bytesClass->attributes["__new__"]     = Value(std::make_shared<BuiltinFunction>("__new__", bytesNew));
    Runtime::bytearrayClass->attributes["__new__"] = Value(std::make_shared<BuiltinFunction>("__new__", bytearrayNew));
    Runtime::rangeClass->attributes["__new__"]     = Value(std::make_shared<BuiltinFunction>("__new__", rangeNew));
}

Value constructClass(const Value::ClassPtr& cls,
                     const std::vector<Value>& args,
                     const Kwargs& kwargs,
                     const std::shared_ptr<Environment>& env) {

    // Конструируем ли мы класс (cls — метакласс)? Тогда по пути __new__
    // нужно проставить метакласс результата и вызвать метаклассовый __init__.
    const bool isMetaConstruction =
        PythonException::isSubclass(cls, Runtime::typeClass);

    // Обобщённое конструирование: если у класса есть __new__ — конструируем через него.
    // Ловим AttributeError только на самом поиске __new__ (его отсутствие), а не
    // на вызовах пользовательского кода, чтобы не глотать настоящие ошибки.
    Value newMethod;
    bool hasNew = false;

    try {
        newMethod = findAttrInHierarchy(cls, "__new__");
        hasNew = true;
    } catch (const AttributeErrorException&) {
        // у класса нет __new__ — обычное создание экземпляра ниже
    }

    if (hasNew) {

        // Питоновскому __new__ передаём класс первым аргументом: __new__(cls, ...).
        // Встроенные __new__ — фабрики без cls, их вызываем как есть.
        Value constructed;

        if (newMethod.isFunction()) {

            std::vector<Value> newArgs;
            newArgs.reserve(args.size() + 1);
            newArgs.push_back(Value(cls));
            newArgs.insert(newArgs.end(), args.begin(), args.end());

            constructed = call(newMethod, newArgs, kwargs, env);
        }
        else {
            constructed = call(newMethod, args, kwargs, env);
        }

        // Путь метакласса: закрепляем метакласс и зовём его __init__.
        if (isMetaConstruction && constructed.isClass()) {

            const auto newClass = constructed.asClass();

            if (!newClass->metaclass && cls != Runtime::typeClass) {
                newClass->metaclass = cls;
            }

            // Метаклассовый __init__(cls_result, name, bases, ns), если он задан
            // пользователем (type/object своего __init__ здесь не имеют).
            Value metaInit;
            bool hasInit = false;

            try {
                metaInit = findAttrInHierarchy(cls, "__init__");
                hasInit = true;
            } catch (const AttributeErrorException&) {}

            if (hasInit && metaInit.isFunction()) {

                std::vector<Value> initArgs;
                initArgs.reserve(args.size() + 1);
                initArgs.push_back(constructed);
                initArgs.insert(initArgs.end(), args.begin(), args.end());

                call(metaInit, initArgs, kwargs, env);
            }
        }

        return constructed;
    }

    const auto instance = std::make_shared<InstanceValue>(cls);

    if (PythonException::isSubclass(cls, Runtime::baseExceptionClass)) {
        instance->fields["args"] = Value(
            std::make_shared<TupleValue>(args)
        );
    }

    try {
        const Value init = getAttrValue(Value(instance), "__init__");
        call(init, args, kwargs, env);
    } catch (const AttributeErrorException&) {

        if (!args.empty()) {
            throw TypeErrorException("Class takes no arguments");
        }
    }

    return Value(instance);
}

Value callBoundMethod(const Value::BoundMethodPtr &bm,
                      const std::vector<Value> &args,
                      const Kwargs& kwargs) {
    std::vector<Value> newArgs;

    // self
    newArgs.emplace_back(bm->self);

    // остальные аргументы
    newArgs.insert(newArgs.end(), args.begin(), args.end());

    if (const auto f =
        std::get_if<Value::FunctionPtr>(&bm->callable.data)) {

        const auto local = std::make_shared<Environment>((*f)->closure);

        local->set("__class__", Value(bm->ownerClass));

        return callFunction(*f, newArgs, kwargs, local);
    }


    if (const auto b =
       std::get_if<Value::BuiltinFunctionPtr>(&bm->callable.data)) {
        return (*b)->func(newArgs, kwargs, nullptr);
    }

    throw TypeErrorException("object is not callable");
}

QByteArray constructBytesData(const std::vector<Value> &args, const Kwargs &kwargs) {

    std::optional<QString> encoding;

        //TODO: пока не поддерживается
        std::optional<QString> errors;

        for (const auto& [name, value] : kwargs) {

            if (name == "encoding") {

                encoding = value.asString("bytes")->toString();

            } else if (name == "errors") {

                //TODO: пока не поддерживается
                errors = value.asString("bytes")->toString();

            } else {

                throw ValueErrorException(
                "Unknown keyword argument: " + name
                );
            }
        }

        expectArgsRange(args, 0, 2, "bytes");

        if (args.empty()) {
            return {};
        }

        const Value& obj = args[0];

        if (obj.isBytes()) {

            if (encoding.has_value()) {

                throw TypeErrorException("encoding without a string argument");
            }

            return obj.asBytes("bytes")->bytes();
        }

        if (obj.isByteArray()) {

            if (encoding.has_value()) {
                throw TypeErrorException("encoding without a string argument");
            }

            return obj.asByteArray("bytes")->bytes();
        }

        try {

            Value bytesMethod = getAttrValue(obj, "__bytes__");

            Value result = call(bytesMethod, {}, {}, nullptr);

            if (!result.isBytes()) {

                throw TypeErrorException("__bytes__ returned non-bytes");
            }

            return result.asBytes()->bytes();

        }
        catch (const AttributeErrorException& e) {}

        if (obj.isString()) {

            QString actualEncoding;

            if (args.size() >= 2) {

                actualEncoding = args[1].asString("bytes")->toString();

            } else if (encoding.has_value()) {

                actualEncoding = *encoding;
                //TODO: if (errors.has_value()) {}
            } else {

                throw TypeErrorException("string argument without an encoding");
            }

            // TODO: пока поддерживается только utf-8
            if (actualEncoding != "utf-8" && actualEncoding != "utf8") {

                throw LookupErrorException("unknown encoding");
            }

            return  obj.toString().toUtf8();
        }

        if (obj.isBigInt() || obj.isBool()) {

            auto count = obj.toBigInt();

            if (count < 0) {
                throw;
            }

            return {count.convert_to<long long>(), '\0'};
        }

        if (obj.isIterable() || supportsIter(obj)) {

            QByteArray result;

            Value iterMethod = getAttrValue(obj, "__iter__");

            Value iterObj = call(iterMethod, {}, {}, nullptr);

            if (!std::holds_alternative<Value::IteratorPtr>(iterObj.data)) {
                throw TypeErrorException("__iter__ returned non-iterator");
            }

            auto iterator = std::get<Value::IteratorPtr>(iterObj.data);

            while (iterator->hasNext()) {

                Value item = iterator->next();

                auto value = item.toBigInt();

                if (value < 0 || value > 255) {
                    throw ValueErrorException("bytes must be in range(0, 256)");
                }

                result.append(
                    static_cast<char>(
                        value.convert_to<int>()
                    )
                );
            }

            return result;
        }

        throw TypeErrorException("cannot convert object to bytes");


}
