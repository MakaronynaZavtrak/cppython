//
// Created by semyo on 27.06.2026.
//
#include "..//exception/PythonException.h"

PythonException::PythonException(
    QString typeName,
    QString message)
    : typeName(std::move(typeName)),
      message(std::move(message)) {

    cachedWhat =
        QString("%1: %2")
            .arg(this->typeName, this->message)
            .toStdString();
}

const char* PythonException::what() const noexcept {
    return cachedWhat.c_str();
}

const QString& PythonException::getTypeName() const {
    return typeName;
}

const QString& PythonException::getMessage() const {
    return message;
}