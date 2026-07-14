//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_INDEXNODE_H
#define CPPYTHON_INDEXNODE_H

#include "../ASTNode.h"

class IndexNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> object;
    std::shared_ptr<ASTNode> index;

    int objectEndColumn;

    IndexNode(std::shared_ptr<ASTNode> object,
              std::shared_ptr<ASTNode> index);

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;
};
#endif //CPPYTHON_INDEXNODE_H