#include "CallRuntime.h"

#include "BoundMethod.h"
#include "BytesValue.h"
#include "ClassMethodValue.h"
#include "Environment.h"
#include "FunctionValue.h"
#include "Parser.h"
#include "StaticMethodValue.h"
#include "StrValue.h"
#include "Value.h"
//
// Created by semyo on 03.05.2026.
//
Value call(const Value& callee,
           const std::vector<Value>& args,
           const Kwargs& kwargs,
           const std::shared_ptr<Environment>& env)
{

    if (std::holds_alternative<Value::BuiltinFunctionPtr>(callee.data)) {
        const auto fn = std::get<Value::BuiltinFunctionPtr>(callee.data);
        return fn->func(args, kwargs, env);
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

    throw std::runtime_error("Object is not callable");
}

Value callFunction(const Value::FunctionPtr& func,
                   const std::vector<Value>& args,
                   const Kwargs& kwargs,
                   const std::shared_ptr<Environment>& envOverride = nullptr)
{

    const auto local = std::make_shared<Environment>(func->closure);

    if (envOverride) {
        for (const auto& [k, v] : envOverride->variables) {
            local->set(k, v);
        }
    }

    std::unordered_set<QString> assigned;

    // позиционные аргументы
    for (size_t i = 0; i < args.size(); ++i) {

        if (i >= func->params.size()) {
            throw std::runtime_error("Too many positional arguments");
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
                    throw std::runtime_error(
                    "Multiple values for argument: "+ name.toStdString()
                    );
                }

                local->set(name, value);

                assigned.insert(name);

                found = true;
                break;
            }
        }

        if (!found) {
            throw std::runtime_error("Unknown keyword argument: " + name.toStdString());
        }
    }

    // отсутствие аргументов
    for (const auto& param : func->params) {

        if (!assigned.count(param.name)) {
            throw std::runtime_error("Missing argument: " + param.name.toStdString());
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

Value constructClass(const Value::ClassPtr& cls,
                     const std::vector<Value>& args,
                     const Kwargs& kwargs,
                     const std::shared_ptr<Environment>& env) {

    if (cls == Runtime::strClass) {

        if (args.empty()) {
            return Value("");
        }

        if (args.size() > 1) {
            throw std::runtime_error(
                "str() takes at most 1 argument"
            );
        }

        return Value(args[0].toString());
    }

    if (cls == Runtime::bytesClass) {

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

                throw std::runtime_error(
                    "Unknown keyword argument: "
                    + name.toStdString()
                );
            }
        }

        if (args.empty()) {

            return Value(
                std::make_shared<BytesValue>(
                    QByteArray()
                )
            );
        }

        if (args.size() > 2) {

            throw std::runtime_error(
                "bytes() takes at most 2 arguments"
            );
        }

        const Value& obj = args[0];

        if (obj.isBytes()) {

            if (encoding.has_value()) {

                throw std::runtime_error(
                    "TypeError: encoding without a string argument"
                );
            }

            return obj;
        }

        if (obj.isString()) {

            QString actualEncoding;

            if (args.size() >= 2) {

                actualEncoding =
                    args[1]
                        .asString("bytes")
                        ->toString();

            } else if (encoding.has_value()) {

                actualEncoding = *encoding;
            //TODO: if (errors.has_value()) {}
            } else {

                throw std::runtime_error(
                    "TypeError: string argument without an encoding"
                );
            }

            // TODO: пока поддерживается только utf-8
            if (
                actualEncoding != "utf-8" &&
                actualEncoding != "utf8"
            ) {

                throw std::runtime_error(
                    "LookupError: unknown encoding"
                );
            }

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

    const auto instance = std::make_shared<InstanceValue>(cls);

    try {
        const Value init = getAttrValue(Value(instance), "__init__");
        call(init, args, kwargs, env);
    } catch (...) {
        if (!args.empty()) {
            throw std::runtime_error("Class takes no arguments");
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

    throw std::runtime_error("Invalid bound method callable");
}