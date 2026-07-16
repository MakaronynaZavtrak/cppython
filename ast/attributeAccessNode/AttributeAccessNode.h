//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_ATTRIBUTEACCESSNODE_H
#define CPPYTHON_ATTRIBUTEACCESSNODE_H

#include "../ASTNode.h"

class AttributeAccessNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> object;
    QString attr;

    int objectEndColumn;

    AttributeAccessNode(std::shared_ptr<ASTNode> object, QString attr);

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;
};
#endif //CPPYTHON_ATTRIBUTEACCESSNODE_H