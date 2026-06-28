//
// Created by semyo on 18.05.2026.
//

#ifndef CPPYTHON_STOPITERATIONEXCEPTION_H
#define CPPYTHON_STOPITERATIONEXCEPTION_H
#include "PythonException.h"

class StopIterationException final : public PythonException {
public:
    explicit StopIterationException()
        : PythonException("StopIteration", "") {}
};
#endif //CPPYTHON_STOPITERATIONEXCEPTION_H