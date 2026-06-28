//
// Created by semyo on 27.06.2026.
//

#ifndef CPPYTHON_RUNTIMEERROREXCEPTION_H
#define CPPYTHON_RUNTIMEERROREXCEPTION_H
#include "PythonException.h"

class RuntimeErrorException : public PythonException {
public:
    explicit RuntimeErrorException(const QString& msg)
        : PythonException("RuntimeError", msg) {}
};
#endif //CPPYTHON_RUNTIMEERROREXCEPTION_H