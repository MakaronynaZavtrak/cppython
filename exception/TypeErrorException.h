//
// Created by semyo on 27.06.2026.
//

#ifndef CPPYTHON_TYPEERROREXCEPTION_H
#define CPPYTHON_TYPEERROREXCEPTION_H
#include "PythonException.h"
#include "../runtime/Runtime.h"

class TypeErrorException : public PythonException {
public:
    explicit TypeErrorException(const QString& msg)
        : PythonException(Runtime::typeErrorClass, msg) {}
};
#endif //CPPYTHON_TYPEERROREXCEPTION_H