//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_INDEXASSIGNNODE_H
#define CPPYTHON_INDEXASSIGNNODE_H

#include "../ASTNode.h"

class IndexAssignNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> object;
    std::shared_ptr<ASTNode> index;
    std::shared_ptr<ASTNode> value;

    IndexAssignNode(std::shared_ptr<ASTNode> object,
                    std::shared_ptr<ASTNode> index,
                    std::shared_ptr<ASTNode> value);

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] bool shouldPrint() const override;
};
#endif //CPPYTHON_INDEXASSIGNNODE_H