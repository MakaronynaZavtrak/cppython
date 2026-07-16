//
// Created by semyo on 28.06.2026.
//

#ifndef CPPYTHON_NAMEERROREXCEPTION_H
#define CPPYTHON_NAMEERROREXCEPTION_H
#include "PythonException.h"
#include "../runtime/Runtime.h"

class NameErrorException : public PythonException {
public:
    explicit NameErrorException(const QString& msg)
        : PythonException(Runtime::nameErrorClass, msg) {}
};
#endif //CPPYTHON_NAMEERROREXCEPTION_H