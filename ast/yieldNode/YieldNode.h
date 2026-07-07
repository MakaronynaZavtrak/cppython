//
// Created by semyo on 07.07.2026.
//

#ifndef CPPYTHON_YIELDNODE_H
#define CPPYTHON_YIELDNODE_H

#include "../ASTNode.h"

class YieldNode : public ASTNode {
    std::shared_ptr<ASTNode> valueExpr; // может быть nullptr — голый "yield"

public:
    explicit YieldNode(std::shared_ptr<ASTNode> valueExpr)
        : valueExpr(std::move(valueExpr)) {}

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] bool shouldPrint() const override { return false; }

    [[nodiscard]] bool containsYield() const override { return true; }
};
#endif //CPPYTHON_YIELDNODE_H