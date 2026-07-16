//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_STARREDNODE_H
#define CPPYTHON_STARREDNODE_H

#include "../ASTNode.h"

class StarredNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> value;

    explicit StarredNode(std::shared_ptr<ASTNode> value);

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;
};
#endif //CPPYTHON_STARREDNODE_H