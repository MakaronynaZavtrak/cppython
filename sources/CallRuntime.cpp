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

#include "../exception/AttributeErrorException.h"
#include "../exception/LookupErorException.h"
#include "../exception/TypeErrorException.h"
#include "../exception/ValueErrorException.h"
#include "../runtime/ArgValidation.h"

//
// Created by semyo on 03.05.2026.
//
Value call(const Value& callee,
           const std::vector<Value>& args,
           const Kwargs& kwargs,
           const std::shared_ptr<Environment>& env) {

    if (callee.isBuiltinFunction()) {

        return callee.asBuiltinFunction()->func(args, kwargs, env);
    }

    if (const auto f = std::get_if<Value::FunctionPtr>(&callee.data)) {
        return callFunction(*f, args, kwargs, env);
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

    std::unordered_set<QString> assigned;

    // позиционные аргументы
    for (size_t i = 0; i < args.size(); ++i) {

        if (i >= func->params.size()) {
            throw ValueErrorException("Too many positional arguments");
        }

        const QString& paramName = func->params[i].name;

        local->set(paramName, args[i]);
        assigned.insert(paramName);
    }

    // именованные аргументы
    for (const auto& [name, value] : kwargs) {

        bool found = false;

        for (const auto& param : func->params) {

            if (param.name == name) {

                if (assigned.count(name)) {
                    throw ValueErrorException("multiple values for argument " + name);
                }

                local->set(name, value);

                assigned.insert(name);

                found = true;
                break;
            }
        }

        if (!found) {
            throw ValueErrorException("Unknown keyword argument: " + name);
        }
    }

    // отсутствие аргументов
    for (const auto& param : func->params) {

        if (!assigned.count(param.name)) {
            throw TypeErrorException("Missing argument: " + param.name);
        }
    }

    try {
        Value result;

        for (const auto& stmt : func->body) {
            result = stmt->eval(local);
        }

        return result;
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

Value constructClass(const Value::ClassPtr& cls,
                     const std::vector<Value>& args,
                     const Kwargs& kwargs,
                     const std::shared_ptr<Environment>& env) {

    if (cls == Runtime::strClass) {

        expectArgsRange(args, 0, 1, "str");

        if (args.empty()) {
            return Value("");
        }

        return Value(args[0].toString());
    }

    if (cls == Runtime::bytesClass) {

        return Value(
            std::make_shared<BytesValue>(
                constructBytesData(args, kwargs)
            )
        );

    }

    if (cls == Runtime::bytearrayClass) {

        return Value(
            std::make_shared<ByteArrayValue>(
                constructBytesData(args, kwargs)
            )
        );
    }

    const auto instance = std::make_shared<InstanceValue>(cls);

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
            return QByteArray();
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

            return QByteArray(count.convert_to<long long>(), '\0');
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
