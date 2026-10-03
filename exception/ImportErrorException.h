//
// Created by semyo on 03.10.2026.
//

#ifndef CPPYTHON_IMPORTERROREXCEPTION_H
#define CPPYTHON_IMPORTERROREXCEPTION_H
#include "PythonException.h"

class ImportErrorException : public PythonException {
public:
    explicit ImportErrorException(const QString& msg)
        : PythonException(Runtime::importErrorClass, msg) {}
};
#endif //CPPYTHON_IMPORTERROREXCEPTION_H