//
// Created by semyo on 03.10.2026.
//

#ifndef CPPYTHON_MODULEREGISTRY_H
#define CPPYTHON_MODULEREGISTRY_H
#include "Value.h"

Value::ModulePtr importBuiltinModule(const QString& name);
#endif //CPPYTHON_MODULEREGISTRY_H