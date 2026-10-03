//
// Created by semyo on 03.10.2026.
//

#ifndef CPPYTHON_MODULENOTFOUNDERROREXCEPTION_H
#define CPPYTHON_MODULENOTFOUNDERROREXCEPTION_H
#include "PythonException.h"

class ModuleNotFountErrorException : public PythonException {
public:
    explicit ModuleNotFountErrorException(const QString& msg)
        : PythonException(Runtime::moduleNotFoundErrorClass, msg) {}
};
#endif //CPPYTHON_MODULENOTFOUNDERROREXCEPTION_H