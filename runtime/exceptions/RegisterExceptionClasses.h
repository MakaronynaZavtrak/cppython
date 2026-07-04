//
// Created by semyo on 28.06.2026.
//

#ifndef CPPYTHON_EXCEPTIONCLASSES_H
#define CPPYTHON_EXCEPTIONCLASSES_H
#include <memory>

#include "CallRuntime.h"
#include "Value.h"

class Environment;

void registerExceptionClasses(const std::shared_ptr<Environment>& env);

Value baseExceptionInit(
    const std::vector<Value>& args,
    const Kwargs& kwargs,
    const std::shared_ptr<Environment>&);

Value baseExceptionStr(
    const std::vector<Value>& args,
    const Kwargs& kwargs,
    const std::shared_ptr<Environment>&);

Value keyErrorStr(
    const std::vector<Value>& args,
    const Kwargs& kwargs,
    const std::shared_ptr<Environment>&);

#endif //CPPYTHON_EXCEPTIONCLASSES_H