//
// Created by semyo on 27.06.2026.
//
#include "..//exception/PythonException.h"

#include "ClassValue.h"

PythonException::PythonException(
    const Value::ClassPtr& klass,
    QString message)
    : klass(klass),
      message(std::move(message)) {

    cachedWhat =
    QString("%1: %2")
        .arg(
            klass ? klass->name
                  : "<unknown exception>",
            message)
        .toStdString();
}

const char* PythonException::what() const noexcept {
    return cachedWhat.c_str();
}

const Value::ClassPtr & PythonException::getClass() const {
    return klass;
}

const QString& PythonException::getTypeName() const {
    return klass->name;
}

const QString& PythonException::getMessage() const {
    return message;
}