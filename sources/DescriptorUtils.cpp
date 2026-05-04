//
// Created by semyo on 03.05.2026.
#include "DescriptorUtils.h"
#include "BoundMethod.h"

Value applyDescriptor(const Value& val,
                      const Value::InstancePtr &instance,
                      const Value::ClassPtr &owner) {

    if (std::holds_alternative<Value::FunctionPtr>(val.data)) {
        const auto func = std::get<Value::FunctionPtr>(val.data);
        // если доступ через экземпляр, то bound method
        if (instance) {
            return Value(std::make_shared<BoundMethod>(func, instance, owner));
        }
        // если доступ через класс, то просто функция
        return val;
    }

    return val;
}