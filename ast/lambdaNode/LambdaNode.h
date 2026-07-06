//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_LAMBDANODE_H
#define CPPYTHON_LAMBDANODE_H

#include "Param.h"
#include "../ASTNode.h"

class LambdaNode final : public ASTNode {
public:

    std::vector<Param> params;
    std::shared_ptr<ASTNode> body;

    LambdaNode(std::vector<Param> params,
        std::shared_ptr<ASTNode> body);

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] bool shouldPrint() const override;

};
#endif //CPPYTHON_LAMBDANODE_H