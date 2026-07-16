//
// Created by semyo on 12.05.2026.
//
#include "DescriptorUtils.h"

#include <variant>

#include "BoundMethod.h"
#include "CallRuntime.h"
#include "ClassMethodValue.h"
#include "ClassUtils.h"
#include "FunctionValue.h"
#include "PropertyValue.h"
#include "StaticMethodValue.h"
#include "../exception/AttributeErrorException.h"

bool DescriptorUtils::hasGet(const Value& descriptor) {

    if (descriptor.isFunction())
        return true;

    if (descriptor.isBuiltinFunction())
        return true;

    if (descriptor.isProperty())
        return true;

    if (descriptor.isStaticMethod())
        return true;

    if (descriptor.isClassMethod())
        return true;

    // user-defined descriptor
    try {
        getAttrValue(descriptor, "__get__");
        return true;
    }
    catch (...) {
        return false;
    }
}

Value DescriptorUtils::callGet(const Value& descriptor,
              const Value& instance,
              const Value::ClassPtr& owner) {

    if (const auto f =
    std::get_if<Value::FunctionPtr>(&descriptor.data)) {

        return (*f)->get(instance, owner);
    }

    if (const auto bf =
        std::get_if<Value::BuiltinFunctionPtr>(&descriptor.data)) {

        if (instance.isNone()) {
            return descriptor;
        }

        return Value(
            std::make_shared<BoundMethod>(
                Value(*bf),
                instance,
                owner
            )
        );
        }

    if (const auto p =
        std::get_if<Value::PropertyPtr>(&descriptor.data)) {

        return (*p)->get(instance, owner);
        }

    if (const auto sm =
        std::get_if<Value::StaticMethodPtr>(&descriptor.data)) {

        return (*sm)->get(instance, owner);
        }

    if (const auto cm =
        std::get_if<Value::ClassMethodPtr>(&descriptor.data)) {

        return (*cm)->get(instance, owner);
        }

    // user-defined descriptor

    const Value getter =
        getAttrValue(descriptor, "__get__");

    std::vector<Value> args;

    if (!instance.isNone())
        args.emplace_back(instance);
    else
        args.emplace_back(); // None

    args.emplace_back(owner);

    return call(getter, args, {}, nullptr);
}

bool DescriptorUtils::hasSet(const Value& descriptor) {

    if (descriptor.isProperty()) {

        const auto& prop = descriptor.asProperty();
        return prop->fset != nullptr;
    }

    try {
        getAttrValue(descriptor, "__set__");
        return true;
    }
    catch (...) {
        return false;
    }
}

void DescriptorUtils::callSet(const Value& descriptor,
             const Value& instance,
             const std::shared_ptr<ClassValue>& owner,
             const Value& value) {

    if (descriptor.isProperty()) {

        const auto& prop = descriptor.asProperty();

        if (!prop->fset) {
            throw AttributeErrorException("can't set attribute");
        }

        const auto bound =
            std::make_shared<BoundMethod>(
                Value(prop->fset),
                instance,
                owner
            );

        call(Value(bound), { value }, {}, nullptr);

        return;
    }

    const Value setter = getAttrValue(descriptor, "__set__");

    call(setter, { instance, value }, {}, nullptr);
}