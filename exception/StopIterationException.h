//
// Created by semyo on 18.05.2026.
//

#ifndef CPPYTHON_STOPITERATIONEXCEPTION_H
#define CPPYTHON_STOPITERATIONEXCEPTION_H
#include "PythonException.h"
#include "../runtime/Runtime.h"
#include "InstanceValue.h"

class StopIterationException final : public PythonException {
public:
    explicit StopIterationException()
        : PythonException(Runtime::stopIterationClass, "") {}

    explicit StopIterationException(const Value& returnValue)
        : PythonException(Runtime::stopIterationClass, "") {
        getInstance()->fields["value"] = returnValue;
    }
};
#endif //CPPYTHON_STOPITERATIONEXCEPTION_H