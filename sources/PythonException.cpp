//
// Created by semyo on 27.06.2026.
//
#include "..//exception/PythonException.h"

#include "ClassValue.h"
#include "InstanceValue.h"
#include "TupleValue.h"

PythonException::PythonException(
    const Value::ClassPtr& klass,
    QString message)
    : klass(klass),
      message(std::move(message)) {

    instance = std::make_shared<InstanceValue>(klass);

    instance->fields["message"] = Value(message);
    instance->fields["args"] = Value(
        std::make_shared<TupleValue>(
            std::vector{ Value(message) }
        )
    );

    instance->exceptionMessage = this->message;

    cachedWhat =
        QString("%1: %2")
            .arg(
                klass
                    ? klass->name
                    : "<unknown exception>",
                this->message
            )
            .toStdString();
}

const char* PythonException::what() const noexcept {
    return cachedWhat.c_str();
}

const Value::ClassPtr & PythonException::getClass() const {
    return klass;
}

const std::shared_ptr<InstanceValue>& PythonException::getInstance() const {
    return instance;
}

const QString& PythonException::getTypeName() const {
    return klass->name;
}

const QString& PythonException::getMessage() const {
    return message;
}

bool PythonException::isSubclass(
    const Value::ClassPtr& child,
    const Value::ClassPtr& parent) {

    if (!child || !parent)
        return false;

    if (child == parent)
        return true;

    for (const auto& base : child->bases) {

        if (isSubclass(base, parent))
            return true;
    }

    return false;
}
