//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_FORNODE_H
#define CPPYTHON_FORNODE_H

#include "../ASTNode.h"

class ForNode : public ASTNode {
public:

    QString varName;

    std::shared_ptr<ASTNode> iterable;

    std::vector<std::shared_ptr<ASTNode>> body;

    ForNode(QString varName,
            std::shared_ptr<ASTNode> iterable,
            std::vector<std::shared_ptr<ASTNode>> body);

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] bool shouldPrint() const override;

    [[nodiscard]] bool containsYield() const override;

    [[nodiscard]] Value evalResumable(EnvPtr env, ResumeContext& ctx) const override;
};
#endif //CPPYTHON_FORNODE_H