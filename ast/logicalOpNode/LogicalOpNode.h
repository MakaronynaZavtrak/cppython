//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_LOGICALOPNODE_H
#define CPPYTHON_LOGICALOPNODE_H

#include "../ASTNode.h"

class LogicalOpNode : public ASTNode {

    QString op;
    std::shared_ptr<ASTNode> left;
    std::shared_ptr<ASTNode> right;

public:

    LogicalOpNode(std::shared_ptr<ASTNode> left, QString op,  std::shared_ptr<ASTNode> right);

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;
};
#endif //CPPYTHON_LOGICALOPNODE_H