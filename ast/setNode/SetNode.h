//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_SETNODE_H
#define CPPYTHON_SETNODE_H

#include "../ASTNode.h"

class SetNode : public ASTNode {
public:
    std::vector<std::shared_ptr<ASTNode>> elements;

    explicit SetNode(std::vector<std::shared_ptr<ASTNode>> elements);

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;
};
#endif //CPPYTHON_SETNODE_H