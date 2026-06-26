//
// Created by semyo on 04.05.2026.
//

#ifndef CPPYTHON_BUILTINFUNCTION_H
#define CPPYTHON_BUILTINFUNCTION_H
#include <functional>
#include "ReprMixin.h"

class InstanceValue;
class ClassValue;
class Value;
class Environment;

class BuiltinFunction : public ReprMixin, public std::enable_shared_from_this<BuiltinFunction> {
public:
    using FuncType = std::function<Value(const std::vector<Value>&,
        const std::vector<std::pair<QString, Value>>&,
        const std::shared_ptr<Environment>&)>;

    FuncType func;
    QString name;

    BuiltinFunction(QString name, FuncType func)
        : func(std::move(func)), name(std::move(name)) {}

    static void registerBuiltins(const std::shared_ptr<Environment>&);

    Value get(const std::shared_ptr<InstanceValue>&, const std::shared_ptr<ClassValue>&);

    [[nodiscard]] QString toString() const override;
};
#endif //CPPYTHON_BUILTINFUNCTION_H