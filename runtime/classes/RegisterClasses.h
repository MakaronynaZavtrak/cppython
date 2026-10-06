//
// Created by semyo on 30.06.2026.
//

#ifndef CPPYTHON_REGISTERCORECLASSES_H
#define CPPYTHON_REGISTERCORECLASSES_H
#include <memory>
class Environment;

void registerObjectClass(const std::shared_ptr<Environment>& env);

void registerStringClass(const std::shared_ptr<Environment>& env);

void registerBytesClass(const std::shared_ptr<Environment>& env);

void registerByteArrayClass(const std::shared_ptr<Environment>& env);

void registerRangeClass(const std::shared_ptr<Environment>& env);

void registerBuiltinTypeClasses(const std::shared_ptr<Environment>& env);
#endif //CPPYTHON_REGISTERCORECLASSES_H