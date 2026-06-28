//
// Created by semyo on 28.06.2026.
//

#ifndef CPPYTHON_LOOKUPEROREXCEPTION_H
#define CPPYTHON_LOOKUPEROREXCEPTION_H
#include "PythonException.h"

class LookupErrorException : public PythonException {
public:
    explicit LookupErrorException(const QString& msg)
        : PythonException("LookupError", msg) {}
};
#endif //CPPYTHON_LOOKUPEROREXCEPTION_H