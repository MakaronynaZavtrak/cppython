//
// Created by semyo on 18.05.2026.
//

#ifndef CPPYTHON_STOPITERATIONEXCEPTION_H
#define CPPYTHON_STOPITERATIONEXCEPTION_H
#include "PythonException.h"
#include "../runtime/Runtime.h"

class StopIterationException final : public PythonException {
public:
    explicit StopIterationException()
        : PythonException(Runtime::stopIterationClass, "") {}
};
#endif //CPPYTHON_STOPITERATIONEXCEPTION_H