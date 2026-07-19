//
// Created by semyo on 19.07.2026.
//

#ifndef CPPYTHON_ZERODIVISIONERROREXCEPTION_H
#define CPPYTHON_ZERODIVISIONERROREXCEPTION_H
#include "PythonException.h"
#include "../runtime/Runtime.h"

class ZeroDivisionErrorException : public PythonException {
public:
    explicit ZeroDivisionErrorException(const QString& msg)
        : PythonException(Runtime::zeroDivisionErrorClass, msg) {}
};
#endif //CPPYTHON_ZERODIVISIONERROREXCEPTION_H