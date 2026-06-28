//
// Created by semyo on 28.06.2026.
//

#ifndef CPPYTHON_SYNTAXERROREXCEPTION_H
#define CPPYTHON_SYNTAXERROREXCEPTION_H
#include "PythonException.h"

class SyntaxErrorException : public PythonException {
public:
    explicit SyntaxErrorException(const QString& msg)
        : PythonException("SyntaxError", msg) {}
};
#endif //CPPYTHON_SYNTAXERROREXCEPTION_H