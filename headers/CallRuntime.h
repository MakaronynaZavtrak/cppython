//
// Created by semyo on 03.05.2026.
//

#ifndef CPPYTHON_CALLRUNTIME_H
#define CPPYTHON_CALLRUNTIME_H
#pragma once

#include "Environment.h"
#include "Value.h"

using Kwargs = std::vector<std::pair<QString, Value>>;

Value call(const Value&,
           const std::vector<Value>&,
           const Kwargs&,
           const std::shared_ptr<Environment>&);

Value callFunction(const Value::FunctionPtr&,
                   const std::vector<Value>&,
                   const Kwargs&,
                   const std::shared_ptr<Environment>&);

Value constructClass(const Value::ClassPtr&,
                     const std::vector<Value>&,
                     const Kwargs&,
                     const std::shared_ptr<Environment>&);

Value callBoundMethod(const Value::BoundMethodPtr&,
                      const std::vector<Value>&,
                      const Kwargs&);

QByteArray constructBytesData(
    const std::vector<Value>& args,
    const Kwargs& kwargs);

#endif //CPPYTHON_CALLRUNTIME_H