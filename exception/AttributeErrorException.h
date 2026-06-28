//
// Created by semyo on 27.06.2026.
//

#ifndef CPPYTHON_ATTRIBUTEERROREXCEPTION_H
#define CPPYTHON_ATTRIBUTEERROREXCEPTION_H
#include "PythonException.h"

class AttributeErrorException : public PythonException {
public:
    explicit AttributeErrorException(const QString& msg)
        : PythonException("AttributeError", msg) {}
};
#endif //CPPYTHON_ATTRIBUTEERROREXCEPTION_H