//
// Created by semyo on 25.05.2026.
//

#ifndef CPPYTHON_BUILTINMETHODREGISTRY_H
#define CPPYTHON_BUILTINMETHODREGISTRY_H
#include <functional>

#include <QHash>
#include <QString>

#include "Value.h"

#define REGISTER_METHOD(name, fn) \
{ name, BuiltinMethodFactory(fn) }

using BuiltinMethodFactory =
    std::function<Value(const Value&)>;

using MethodMap = QHash<QString, BuiltinMethodFactory>;

#endif //CPPYTHON_BUILTINMETHODREGISTRY_H