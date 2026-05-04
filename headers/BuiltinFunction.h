//
// Created by semyo on 04.05.2026.
//

#ifndef CPPYTHON_BUILTINFUNCTION_H
#define CPPYTHON_BUILTINFUNCTION_H
#include <functional>

class Value;
class Environment;

class BuiltinFunction {
public:
    using FuncType = std::function<Value(const std::vector<Value>&, const std::shared_ptr<Environment>&)>;

    FuncType func;
    QString name;

    BuiltinFunction(QString name, FuncType func)
        : func(std::move(func)), name(std::move(name)) {}

    static void registerBuiltins(const std::shared_ptr<Environment>&);
};
#endif //CPPYTHON_BUILTINFUNCTION_H