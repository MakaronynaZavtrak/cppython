//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_ATTRIBUTEASSIGNNODE_H
#define CPPYTHON_ATTRIBUTEASSIGNNODE_H

#include "../ASTNode.h"

class AttributeAssignNode final : public ASTNode {
public:
    std::shared_ptr<ASTNode> object;
    QString attr;
    std::shared_ptr<ASTNode> valueExpr;

    AttributeAssignNode(std::shared_ptr<ASTNode> object,
                        QString attr,
                        std::shared_ptr<ASTNode> valueExpr);

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] bool shouldPrint() const override;
};
#endif //CPPYTHON_ATTRIBUTEASSIGNNODE_H