#include "ClassUtils.h"
#include "Environment.h"
#include "InstanceValue.h"
#include "Value.h"
//
// Created by semyo on 04.05.2026.
//
void BuiltinFunction::registerBuiltins(const std::shared_ptr<Environment>& env) {
    env->set("super", Value(std::make_shared<BuiltinFunction>(
    "super",
    [](const std::vector<Value>&, const std::shared_ptr<Environment>& local_env) -> Value {

        auto selfVal = local_env->get("self");
        auto clsVal  = local_env->get("__class__");

        auto instance = std::get<Value::InstancePtr>(selfVal.data);
        auto origin   = std::get<Value::ClassPtr>(clsVal.data);

        return Value(std::make_shared<SuperValue>(
            origin,    // currentClass (стартуем с него)
            instance,
            origin     // originClass
        ));
    }
)));

    env->set("hasattr", Value(std::make_shared<BuiltinFunction>(
"hasattr",
[](const std::vector<Value>& args, const std::shared_ptr<Environment>&) -> Value {

    if (args.size() != 2)
        throw std::runtime_error("hasattr expects 2 arguments");

    const auto& obj = args[0];
    QString attr;

    if (std::holds_alternative<QString>(args[1].data)) {
        attr = std::get<QString>(args[1].data);
    } else {
        throw std::runtime_error("hasattr: attribute name must be string");
    }

    try {
        // instance
        if (std::holds_alternative<Value::InstancePtr>(obj.data)) {
            const auto instance = std::get<Value::InstancePtr>(obj.data);
            // 1. поля объекта
            if (instance->fields.contains(attr))
                return Value(true);

            // 2. класс + наследование
            return Value(hasAttr(instance->klass, attr));
        }

        // class
        if (std::holds_alternative<Value::ClassPtr>(obj.data)) {
            return Value(hasAttr(std::get<Value::ClassPtr>(obj.data), attr));
        }

        return Value(false);
    }
    catch (...) {
        return Value(false);
    }
}
)));

}
