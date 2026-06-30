//
// Created by semyo on 28.06.2026.
//

#ifndef CPPYTHON_EXCEPTIONCLASSES_H
#define CPPYTHON_EXCEPTIONCLASSES_H
#include <memory>

class Environment;

void registerExceptionClasses(const std::shared_ptr<Environment>& env);
#endif //CPPYTHON_EXCEPTIONCLASSES_H