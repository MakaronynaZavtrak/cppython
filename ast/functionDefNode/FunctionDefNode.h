//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_FUNCTIONDEFNODE_H
#define CPPYTHON_FUNCTIONDEFNODE_H

#include "Param.h"
#include "../ASTNode.h"

class FunctionDefNode final : public ASTNode {
public:
    QString name;
    std::vector<Param> params;
    std::vector<std::shared_ptr<ASTNode>> body;
    std::vector<std::shared_ptr<ASTNode>> decorators;

    FunctionDefNode(QString name,
                    std::vector<Param> params,
                    std::vector<std::shared_ptr<ASTNode>> body,
                    std::vector<std::shared_ptr<ASTNode>> decorators = {});

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] bool shouldPrint() const override;
};
#endif //CPPYTHON_FUNCTIONDEFNODE_H