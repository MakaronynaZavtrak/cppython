#include "CallRuntime.h"

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
        return callBoundMethod(*b, args, env);

    throw std::runtime_error("Object is not callable");
}

Value callFunction(const Value::FunctionPtr& func,
                   const std::vector<Value>& args,
                   const std::shared_ptr<Environment>& env)
{
    if (args.size() != func->params.size()) {
        throw std::runtime_error("Argument count mismatch");
    }

    const auto local = std::make_shared<Environment>(func->closure);

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

Value callBoundMethod(const Value::BoundMethodPtr& bm,
                      const std::vector<Value>& args,
                      const std::shared_ptr<Environment>& env)
{
    const auto& func = bm->function;
    const auto& instance = bm->instance;

    if (args.size() != func->params.size() - 1) {
        throw std::runtime_error("Argument count mismatch");
    }

    const auto local = std::make_shared<Environment>(func->closure);

    // self
    local->set(func->params[0].name, Value(instance));

    local->set("__class__", Value(func->ownerClass));

    // args
    for (size_t i = 1; i < func->params.size(); ++i) {
        local->set(func->params[i].name, args[i - 1]);
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