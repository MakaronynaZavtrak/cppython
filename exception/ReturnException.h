//
// Created by semyo on 27.06.2026.
//

#ifndef CPPYTHON_RETURNEXCEPTION_H
#define CPPYTHON_RETURNEXCEPTION_H
#include "PythonException.h"
#include "Value.h"

class ReturnException final : public PythonException {
public:
    explicit ReturnException(Value val)
    : PythonException("", ""),
    value(std::move(val)) {}

    [[nodiscard]] Value getValue() const { return value; }

private:
    Value value;
};
#endif //CPPYTHON_RETURNEXCEPTION_H