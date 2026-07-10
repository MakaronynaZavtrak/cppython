//
// Created by semyo on 09.07.2026.
//

#ifndef CPPYTHON_YIELDFROMNODE_H
#define CPPYTHON_YIELDFROMNODE_H

#include "../ASTNode.h"

class YieldFromNode : public ASTNode {
    std::shared_ptr<ASTNode> valueExpr;

public:
    explicit YieldFromNode(std::shared_ptr<ASTNode> valueExpr)
        : valueExpr(std::move(valueExpr)) {}

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] Value evalResumable(EnvPtr env, ResumeContext& ctx) const override;

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] bool shouldPrint() const override { return false; }

    [[nodiscard]] bool containsYield() const override { return true; }

    [[nodiscard]] bool isBareYieldStatement() const override;
};

#endif //CPPYTHON_YIELDFROMNODE_H