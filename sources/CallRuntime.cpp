#include "CallRuntime.h"

#include "BoundMethod.h"
#include "Environment.h"
#include "FunctionValue.h"
#include "Parser.h"
#include "Value.h"
//
// Created by semyo on 03.05.2026.
//
Value call(const Value& callee,
           const std::vector<Value>& args,
           const std::shared_ptr<Environment>& env)
{

    if (std::holds_alternative<Value::BuiltinFunctionPtr>(callee.data)) {
        const auto fn = std::get<Value::BuiltinFunctionPtr>(callee.data);
        return fn->func(args, env);
    }

    if (const auto f = std::get_if<Value::FunctionPtr>(&callee.data))
        return callFunction(*f, args, env);

    if (const auto c = std::get_if<Value::ClassPtr>(&callee.data))
        return constructClass(*c, args, env);

    if (const auto b = std::get_if<Value::BoundMethodPtr>(&callee.data))
        return callBoundMethod(*b, args);

    throw std::runtime_error("Object is not callable");
}

Value callFunction(const Value::FunctionPtr& func,
                   const std::vector<Value>& args,
                   const std::shared_ptr<Environment>& envOverride = nullptr)
{
    if (args.size() != func->params.size()) {
        throw std::runtime_error("Argument count mismatch");
    }

    const auto local = envOverride
        ? envOverride
        : std::make_shared<Environment>(func->closure);

    for (size_t i = 0; i < args.size(); ++i) {
        local->set(func->params[i].name, args[i]);
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
                     const std::shared_ptr<Environment>& env)
{
    const auto instance = std::make_shared<InstanceValue>(cls);

    try {
        const Value init = getAttrValue(Value(instance), "__init__");
        call(init, args, env);
    } catch (...) {
        if (!args.empty()) {
            throw std::runtime_error("Class takes no arguments");
        }
    }

    return Value(instance);
}

Value callBoundMethod(const Value::BoundMethodPtr &bm, const std::vector<Value> &args) {
    std::vector<Value> newArgs;

    // self
    newArgs.emplace_back(bm->instance);

    // остальные аргументы
    newArgs.insert(newArgs.end(), args.begin(), args.end());

    if (const auto f =
        std::get_if<Value::FunctionPtr>(&bm->callable.data)) {
        const auto local = std::make_shared<Environment>((*f)->closure);

        local->set("__class__", Value(bm->ownerClass));

        return callFunction(*f, newArgs, local);
    }


    if (const auto b =
       std::get_if<Value::BuiltinFunctionPtr>(&bm->callable.data)) {
        return (*b)->func(newArgs, nullptr);
    }

    throw std::runtime_error("Invalid bound method callable");
}