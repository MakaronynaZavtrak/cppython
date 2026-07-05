//
// Created by semyo on 27.06.2026.
//
#include <utility>

#include "..//exception/PythonException.h"
#include "../runtime/Runtime.h"

#include "ClassValue.h"
#include "InstanceValue.h"
#include "StrValue.h"
#include "TupleValue.h"

PythonException::PythonException(
    Value::InstancePtr instance)
    : instance(std::move(instance)) {

    if (!Runtime::exceptionStack.empty() &&
        Runtime::exceptionStack.back() != this->instance &&
        !this->instance->fields.contains("__context__")) {

        this->instance->fields["__context__"] =
            Value(Runtime::exceptionStack.back());

        } else if (!this->instance->fields.contains("__context__")) {

            this->instance->fields["__context__"] = Value(); // None
        }

    if (!this->instance->fields.contains("__cause__")) {
        this->instance->fields["__cause__"] = Value(); // None
    }

    if (!this->instance->fields.contains("__suppress_context__")) {
        this->instance->fields["__suppress_context__"] = Value(false);
    }

    QString message = getMessage();

    cachedWhat =
        QString("%1: %2")
            .arg(
                getTypeName(),
                message
            )
            .toStdString();
}

const char* PythonException::what() const noexcept {
    return cachedWhat.c_str();
}

const Value::ClassPtr & PythonException::getClass() const {
    return instance->klass;
}

const std::shared_ptr<InstanceValue>& PythonException::getInstance() const {
    return instance;
}

QString PythonException::getTypeName() const {

    return instance->klass
          ? instance->klass->name
          : "<unknown exception>";
}

QString PythonException::getMessage() const {

    const auto it = instance->fields.find("message");

    if (it == instance->fields.end())
        return "";

    return it.value().asString()->getValue();
}

bool PythonException::isSubclass(
    const Value::ClassPtr& child,
    const Value::ClassPtr& parent) {

    if (!child || !parent)
        return false;

    if (child == parent)
        return true;

    return std::any_of(
        child->bases.begin(),
        child->bases.end(),
        [&](const auto& base) {
            return isSubclass(base, parent);
    });
}

Value::InstancePtr PythonException::makeInstance(
    const Value::ClassPtr &klass,
    const QString &message) {

    auto instance = std::make_shared<InstanceValue>(klass);

    instance->fields["message"] = Value(message);

    instance->fields["args"] =
        Value(
            std::make_shared<TupleValue>(
                std::vector{
                    Value(message)
                }
            )
        );

    return instance;

}
