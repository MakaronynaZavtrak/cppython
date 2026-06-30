//
// Created by semyo on 27.06.2026.
//

#ifndef CPPYTHON_ARITHMETICERROREXCEPTION_H
#define CPPYTHON_ARITHMETICERROREXCEPTION_H
#include "PythonException.h"
#include "../runtime/Runtime.h"

class ArithmeticErrorException : public PythonException {
public:
    explicit ArithmeticErrorException(const QString& msg)
        : PythonException(Runtime::arithmeticErrorClass, msg) {}
};
#endif //CPPYTHON_ARITHMETICERROREXCEPTION_H