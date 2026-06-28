//
// Created by semyo on 27.06.2026.
//

#ifndef CPPYTHON_ARITHMETICERROREXCEPTION_H
#define CPPYTHON_ARITHMETICERROREXCEPTION_H
#include "PythonException.h"

class ArithmeticErrorException : public PythonException {
public:
    explicit ArithmeticErrorException(const QString& msg)
        : PythonException("ArithmeticError", msg) {}
};
#endif //CPPYTHON_ARITHMETICERROREXCEPTION_H