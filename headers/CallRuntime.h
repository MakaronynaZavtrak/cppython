//
// Created by semyo on 03.05.2026.
//

#ifndef CPPYTHON_CALLRUNTIME_H
#define CPPYTHON_CALLRUNTIME_H
#pragma once

#include "Environment.h"
#include "Value.h"
#include "../runtime/Runtime.h"

using Kwargs = std::vector<std::pair<QString, Value>>;

class CallStackGuard {
public:
    explicit CallStackGuard(const QString& functionName, const int sourceId) {
        Runtime::callStack.push_back(TracebackFrame{functionName, sourceId, 0});
    }
    ~CallStackGuard() {
        Runtime::callStack.pop_back();
    }
    CallStackGuard(const CallStackGuard&) = delete;
    CallStackGuard& operator=(const CallStackGuard&) = delete;
};

void bindParams(const std::shared_ptr<Environment>& local,
                const Value::FunctionPtr& func,
                const std::vector<Value>& args,
                const Kwargs& kwargs);

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

bool supportsIter(const Value& obj);

#endif //CPPYTHON_CALLRUNTIME_H