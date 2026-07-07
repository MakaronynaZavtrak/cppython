//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_TRYNODE_H
#define CPPYTHON_TRYNODE_H

#include "../ASTNode.h"

class ExceptionScopeGuard {
public:
    explicit ExceptionScopeGuard(const Value::InstancePtr& instance);

    ~ExceptionScopeGuard();

    ExceptionScopeGuard(const ExceptionScopeGuard&) = delete;
    ExceptionScopeGuard& operator=(const ExceptionScopeGuard&) = delete;
};

class TryNode : public ASTNode {

public:

    struct ExceptClause {
        std::shared_ptr<ASTNode> exceptionExpr;
        QString variableName;    // "" если без as
        std::vector<std::shared_ptr<ASTNode>> body;
    };

private:

    std::vector<std::shared_ptr<ASTNode>> tryBody;

    std::vector<ExceptClause> excepts;

    std::vector<std::shared_ptr<ASTNode>> elseBody;

    std::vector<std::shared_ptr<ASTNode>> finallyBody;

public:

    TryNode(
    std::vector<std::shared_ptr<ASTNode>> tryBody,
    std::vector<ExceptClause> excepts,
    std::vector<std::shared_ptr<ASTNode>> elseBody,
    std::vector<std::shared_ptr<ASTNode>> finallyBody);

    static bool matchesExceptionHandler(const Value& handlerValue, const Value::ClassPtr& excClass);

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] bool shouldPrint() const override;

    [[nodiscard]] bool containsYield() const override;
};
#endif //CPPYTHON_TRYNODE_H