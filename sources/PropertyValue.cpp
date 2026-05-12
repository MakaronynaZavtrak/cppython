#include "PropertyValue.h"

#include <stdexcept>

#include "BoundMethod.h"
#include "CallRuntime.h"
//
// Created by semyo on 05.05.2026.
//
Value PropertyValue::get(const Value& instance,
                         const std::shared_ptr<ClassValue>& owner) const {
    if (!fget) {
        throw std::runtime_error("unreadable attribute");
    }

    // вызываем fget как bound method
    const auto bound = std::make_shared<BoundMethod>(Value(fget), instance, owner);
    return callBoundMethod(bound, {});
}
