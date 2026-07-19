//
// Created by semyo on 28.06.2026.
//
#include "RegisterExceptionClasses.h"

#include "../Runtime.h"

#include "ClassValue.h"
#include "Environment.h"
#include "InstanceValue.h"
#include "TupleValue.h"
#include "../ArgValidation.h"
#include "../../exception/TypeErrorException.h"

static std::shared_ptr<ClassValue> createExceptionClass(
    const QString& name,
    const std::shared_ptr<ClassValue>& base,
    const std::shared_ptr<Environment>& env) {

    auto klass = std::make_shared<ClassValue>(name);

    klass->name = name;

    if (base)
        klass->bases.push_back(base);

    env->set(name, Value(klass));

    return klass;
}

void registerExceptionClasses(const std::shared_ptr<Environment>& env) {

    Runtime::baseExceptionClass =
        createExceptionClass(
            "BaseException",
            Runtime::objectClass,
            env
        );

    Runtime::baseExceptionClass->attributes["__init__"] =
            Value(
                std::make_shared<BuiltinFunction>(
                    "__init__",
                    baseExceptionInit
                )
            );

    Runtime::baseExceptionClass->attributes["__str__"] =
            Value(
                std::make_shared<BuiltinFunction>(
                    "__str__",
                    baseExceptionStr
                )
            );

    Runtime::exceptionClass =
        createExceptionClass(
            "Exception",
            Runtime::baseExceptionClass,
            env
        );

    Runtime::arithmeticErrorClass =
        createExceptionClass(
            "ArithmeticError",
            Runtime::exceptionClass,
            env
        );

    Runtime::overflowErrorClass =
        createExceptionClass(
            "OverflowError",
            Runtime::arithmeticErrorClass,
            env
        );

    Runtime::lookupErrorClass =
        createExceptionClass(
            "LookupError",
            Runtime::exceptionClass,
            env
        );

    Runtime::indexErrorClass =
        createExceptionClass(
            "IndexError",
            Runtime::lookupErrorClass,
            env
        );

    Runtime::keyErrorClass =
        createExceptionClass(
            "KeyError",
            Runtime::lookupErrorClass,
            env
        );

    Runtime::keyErrorClass->attributes["__str__"] =
    Value(
        std::make_shared<BuiltinFunction>(
            "__str__",
            keyErrorStr
        )
    );

    Runtime::runtimeErrorClass =
        createExceptionClass(
            "RuntimeError",
            Runtime::exceptionClass,
            env
        );

    Runtime::nameErrorClass =
        createExceptionClass(
            "NameError",
            Runtime::exceptionClass,
            env
        );

    Runtime::attributeErrorClass =
        createExceptionClass(
            "AttributeError",
            Runtime::exceptionClass,
            env
        );

    Runtime::syntaxErrorClass =
        createExceptionClass(
            "SyntaxError",
            Runtime::exceptionClass,
            env
        );

    Runtime::typeErrorClass =
        createExceptionClass(
            "TypeError",
            Runtime::exceptionClass,
            env
        );

    Runtime::valueErrorClass =
        createExceptionClass(
            "ValueError",
            Runtime::exceptionClass,
            env
        );

    Runtime::unicodeDecodeClass =
        createExceptionClass(
            "UnicodeDecodeError",
            Runtime::valueErrorClass,
            env
        );

    Runtime::stopIterationClass =
        createExceptionClass(
            "StopIteration",
            Runtime::exceptionClass,
            env
        );

    Runtime::generatorExitClass =
    createExceptionClass(
        "GeneratorExit",
        Runtime::baseExceptionClass,
        env
    );

    Runtime::zeroDivisionErrorClass =
    createExceptionClass(
        "ZeroDivisionError",
        Runtime::arithmeticErrorClass,
        env
    );
}

Value baseExceptionInit(
    const std::vector<Value>& args,
    const Kwargs& kwargs,
    const std::shared_ptr<Environment>&) {

    expectNoKwargs(kwargs, "BaseException.__init__");

    if (args.empty())
        throw TypeErrorException("__init__ missing self");

    const auto self = args[0].asInstance("BaseException.__init__");

    std::vector<Value> tupleValues;

    for (size_t i = 1; i < args.size(); ++i)
        tupleValues.push_back(args[i]);

    self->fields["args"] = Value(
        std::make_shared<TupleValue>(tupleValues)
    );

    if (tupleValues.empty()) {

        self->fields["message"] = Value("");

    } else {

        self->fields["message"] =
            Value(tupleValues[0].toString());
    }

    return {};
}

Value baseExceptionStr(
    const std::vector<Value>& args,
    const Kwargs& kwargs,
    const std::shared_ptr<Environment>&) {

    expectNoKwargs(kwargs, "BaseException.__str__");

    if (args.empty())
        throw TypeErrorException("__str__ missing self");

    const auto self = args[0].asInstance("BaseException.__str__");

    const auto it = self->fields.find("args");

    if (it == self->fields.end()) {
        return Value("");
    }

    const auto tuple = it.value().asTuple();

    if (tuple->items.empty()) {
        return Value("");
    }

    if (tuple->items.size() == 1) {
        return Value(tuple->items[0].toString());
    }

    // несколько аргументов — repr всего кортежа
    return Value(it.value().toString());
}

Value keyErrorStr(
    const std::vector<Value>& args,
    const Kwargs& kwargs,
    const std::shared_ptr<Environment>&) {

    expectNoKwargs(kwargs, "KeyError.__str__");

    if (args.empty())
        throw TypeErrorException("__str__ missing self");

    const auto self = args[0].asInstance("KeyError.__str__");

    const auto it = self->fields.find("args");

    if (it == self->fields.end()) {
        return Value("");
    }

    const auto tuple = it.value().asTuple();

    if (tuple->items.empty()) {
        return Value("");
    }

    if (tuple->items.size() == 1) {
        // KeyError.__str__ всегда repr, не str
        return Value(tuple->items[0].repr());
    }

    return Value(it.value().toString());
}
