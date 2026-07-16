//
// Created by semyo on 27.06.2026.
//

#ifndef CPPYTHON_INDEXERROREXCEPTION_H
#define CPPYTHON_INDEXERROREXCEPTION_H
#include "PythonException.h"
#include "../runtime/Runtime.h"

class IndexErrorException : public PythonException {
public:
    explicit IndexErrorException(const QString& msg)
        : PythonException(Runtime::indexErrorClass, msg) {}
};
#endif //CPPYTHON_INDEXERROREXCEPTION_H