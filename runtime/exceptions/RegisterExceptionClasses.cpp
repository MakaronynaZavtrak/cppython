//
// Created by semyo on 28.06.2026.
//
#include "RegisterExceptionClasses.h"

#include "../Runtime.h"

#include "ClassValue.h"
#include "Environment.h"

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
}