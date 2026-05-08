//
// Created by semyo on 03.05.2026.
//

#ifndef CPPYTHON_CLASSUTILS_H
#define CPPYTHON_CLASSUTILS_H
#include "Value.h"

bool hasAttr(const Value::ClassPtr&, const QString&);

Value genericGetAttr(const Value&, const QString&);

Value getAttrValue(const Value&, const QString&);

void setAttrValue(const Value& obj, const QString&, const Value&);

Value getAttrFromSuper(const Value::SuperPtr&, const QString&);

Value findAttrInHierarchy(const Value::ClassPtr&, const QString&);
#endif //CPPYTHON_CLASSUTILS_H