//
// Created by semyo on 28.06.2026.
//

#ifndef CPPYTHON_NAMEERROREXCEPTION_H
#define CPPYTHON_NAMEERROREXCEPTION_H
#include "PythonException.h"

class NameErrorException : public PythonException {
public:
    explicit NameErrorException(const QString& msg)
        : PythonException("NameError", msg) {}
};
#endif //CPPYTHON_NAMEERROREXCEPTION_H