//
// Created by semyo on 24.05.2026.
//

#ifndef CPPYTHON_BUILTINMETHODFACTORY_H
#define CPPYTHON_BUILTINMETHODFACTORY_H
#include <functional>

#include "../../headers/Value.h"

using BuiltinMethodFactory = std::function<Value(const Value&)>;
#endif //CPPYTHON_BUILTINMETHODFACTORY_H