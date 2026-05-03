//
// Created by semyo on 03.05.2026.
#include "DescriptorUtils.h"
#include "FunctionValue.h"

Value applyDescriptor(const Value& val,
                      const Value::InstancePtr &instance,
                      const Value::ClassPtr &owner) {

    if (std::holds_alternative<Value::FunctionPtr>(val.data)) {
        const auto func = std::get<Value::FunctionPtr>(val.data);
        return func->get(instance, owner);
    }

    return val;
}