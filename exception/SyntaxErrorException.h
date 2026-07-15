//
// Created by semyo on 28.06.2026.
//

#ifndef CPPYTHON_SYNTAXERROREXCEPTION_H
#define CPPYTHON_SYNTAXERROREXCEPTION_H
#include "PythonException.h"
#include "../runtime/Runtime.h"

class SyntaxErrorException : public PythonException {
public:

    bool incompleteInput = false;

    explicit SyntaxErrorException(const QString& msg)
        : PythonException(Runtime::syntaxErrorClass, msg) {}
};
#endif //CPPYTHON_SYNTAXERROREXCEPTION_H