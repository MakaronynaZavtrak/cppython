//
// Created by semyo on 03.05.2026.
//

#ifndef CPPYTHON_CLASSUTILS_H
#define CPPYTHON_CLASSUTILS_H
#include "Value.h"

bool hasAttr(const Value::ClassPtr&, const QString&);

Value genericGetAttr(const Value&, const QString&);

Value makeIterMethod(const Value&);

Value getAttrValue(const Value&, const QString&);

void genericSetAttr(const Value&, const QString&, const Value&);

void setAttrValue(const Value&, const QString&, const Value&);

Value getAttrFromSuper(const Value::SuperPtr&, const QString&);

void buildMRO(const Value::ClassPtr&, std::vector<Value::ClassPtr>&);

Value::ClassPtr getObjectClass(const Value& obj);

Value findAttrInHierarchy(const Value::ClassPtr&, const QString&);
#endif //CPPYTHON_CLASSUTILS_H