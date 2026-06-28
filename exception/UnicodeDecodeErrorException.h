//
// Created by semyo on 28.06.2026.
//

#ifndef CPPYTHON_UNICODEDECODEERROREXCEPTION_H
#define CPPYTHON_UNICODEDECODEERROREXCEPTION_H
#include "PythonException.h"

class UnicodeDecodeErrorException : public PythonException {
public:
    explicit UnicodeDecodeErrorException(const QString& msg)
        : PythonException("UnicodeDecodeError", msg) {}
};
#endif //CPPYTHON_UNICODEDECODEERROREXCEPTION_H