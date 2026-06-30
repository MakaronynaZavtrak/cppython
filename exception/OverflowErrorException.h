//
// Created by semyo on 28.06.2026.
//

#ifndef CPPYTHON_OVERFLOWERROREXCEPTION_H
#define CPPYTHON_OVERFLOWERROREXCEPTION_H
#include "PythonException.h"
#include "../runtime/Runtime.h"

class OverflowErrorException : public PythonException {
public:
    explicit OverflowErrorException(const QString& msg)
        : PythonException(Runtime::overflowErrorClass, msg) {}
};
#endif //CPPYTHON_OVERFLOWERROREXCEPTION_H