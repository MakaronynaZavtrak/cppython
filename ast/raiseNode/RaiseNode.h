//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_RAISENODE_H
#define CPPYTHON_RAISENODE_H

#include "../ASTNode.h"

class RaiseNode : public ASTNode {

    std::shared_ptr<ASTNode> exceptionExpr;
    std::shared_ptr<ASTNode> causeExpr;

public:

    explicit RaiseNode(
        std::shared_ptr<ASTNode> exceptionExpr,
        std::shared_ptr<ASTNode> causeExpr = nullptr);

    [[noreturn]] static void raiseException(const Value& value, const Value& cause, bool hasCause);

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;
};
#endif //CPPYTHON_RAISENODE_H