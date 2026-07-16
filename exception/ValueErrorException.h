//
// Created by semyo on 27.06.2026.
//

#ifndef CPPYTHON_VALUEERROREXCEPTION_H
#define CPPYTHON_VALUEERROREXCEPTION_H
#include "PythonException.h"
#include "../runtime/Runtime.h"

class ValueErrorException : public PythonException {
public:
    explicit ValueErrorException(const QString& msg)
        : PythonException(Runtime::valueErrorClass, msg) {}
};
#endif //CPPYTHON_VALUEERROREXCEPTION_H