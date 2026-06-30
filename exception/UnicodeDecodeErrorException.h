//
// Created by semyo on 28.06.2026.
//

#ifndef CPPYTHON_UNICODEDECODEERROREXCEPTION_H
#define CPPYTHON_UNICODEDECODEERROREXCEPTION_H
#include "PythonException.h"
#include "../runtime/Runtime.h"

class UnicodeDecodeErrorException : public PythonException {
public:
    explicit UnicodeDecodeErrorException(const QString& msg)
        : PythonException(Runtime::unicodeDecodeClass, msg) {}
};
#endif //CPPYTHON_UNICODEDECODEERROREXCEPTION_H