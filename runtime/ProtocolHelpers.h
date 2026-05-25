//
// Created by semyo on 25.05.2026.
//

#ifndef CPPYTHON_PROTOCOLHELPERS_H
#define CPPYTHON_PROTOCOLHELPERS_H
#include "CallRuntime.h"
#include "ClassUtils.h"
#include "RuntimeUtils.h"
#include "ArgValidation.h"

inline Value makeIterMethodBuiltin(const Value& obj) {
    return makeIterMethod(obj);
}

template<typename ContainerPtr>
Value makeLenMethodBuiltin(const Value &obj) {

    auto container = extract<ContainerPtr>(obj);

    return makeBuiltin(
        "__len__",

        [container](
                    const std::vector<Value> &args,
                    const Kwargs &,
                    const std::shared_ptr<Environment> &)
    -> Value {

            expectArgs(args, 0, "__len__");

            return Value(Value::BigInt(container->len()));
        }
    );
}

#endif //CPPYTHON_PROTOCOLHELPERS_H