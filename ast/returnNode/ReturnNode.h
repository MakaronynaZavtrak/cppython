//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_RETURNNODE_H
#define CPPYTHON_RETURNNODE_H

#include "../ASTNode.h"

class ReturnNode final : public ASTNode {
public:

    explicit ReturnNode(std::shared_ptr<ASTNode> expr);

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] bool shouldPrint() const override;

private:
    std::shared_ptr<ASTNode> expr;
};
#endif //CPPYTHON_RETURNNODE_H