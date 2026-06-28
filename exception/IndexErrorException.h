//
// Created by semyo on 27.06.2026.
//

#ifndef CPPYTHON_INDEXERROREXCEPTION_H
#define CPPYTHON_INDEXERROREXCEPTION_H
#include "PythonException.h"

class IndexErrorException : public PythonException {
public:
    explicit IndexErrorException(const QString& msg)
        : PythonException("IndexError", msg) {}
};
#endif //CPPYTHON_INDEXERROREXCEPTION_H