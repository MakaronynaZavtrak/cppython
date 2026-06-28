//
// Created by semyo on 27.06.2026.
//

#ifndef CPPYTHON_KEYERROREXCEPTION_H
#define CPPYTHON_KEYERROREXCEPTION_H
#include "PythonException.h"

class KeyErrorException : public PythonException {
public:
    explicit KeyErrorException(const QString& msg)
        : PythonException("KeyError", msg) {}
};
#endif //CPPYTHON_KEYERROREXCEPTION_H