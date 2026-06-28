//
// Created by semyo on 27.06.2026.
//

#ifndef CPPYTHON_VALUEERROREXCEPTION_H
#define CPPYTHON_VALUEERROREXCEPTION_H
#include "PythonException.h"

class ValueErrorException : public PythonException {
public:
    explicit ValueErrorException(const QString& msg)
        : PythonException("ValueError", msg) {}
};
#endif //CPPYTHON_VALUEERROREXCEPTION_H