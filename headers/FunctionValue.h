//
// Created by semyo on 20.04.2026.
//

#ifndef CPPYTHON_FUNCTIONVALUE_H
#define CPPYTHON_FUNCTIONVALUE_H
#include "InstanceValue.h"
#include "Param.h"

class ASTNode;
class Environment;

class FunctionValue : public std::enable_shared_from_this<FunctionValue>, public ReprMixin {
public:
    FunctionValue(
        std::vector<Param> params,
        std::vector<std::shared_ptr<ASTNode>> body,
        const std::shared_ptr<Environment> &env,
        QString name)
            : params(std::move(params)),
            body(std::move(body)),
            closure(env),
            name(std::move(name)) {}

    Value get(const Value&, const std::shared_ptr<ClassValue>&);
    [[nodiscard]] QString toString() const override;

    std::vector<Param> params;
    std::vector<std::shared_ptr<ASTNode>> body;
    std::shared_ptr<Environment> closure;
    QString name;
    std::shared_ptr<ClassValue> ownerClass;
};

#endif //CPPYTHON_FUNCTIONVALUE_H