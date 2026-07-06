//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_DICTPAIRNODE_H
#define CPPYTHON_DICTPAIRNODE_H

#include "../DictElementNode.h"


class DictPairNode : public DictElementNode {
public:

    std::shared_ptr<ASTNode> key;
    std::shared_ptr<ASTNode> value;

    DictPairNode(std::shared_ptr<ASTNode> key,
                std::shared_ptr<ASTNode> value);

    void apply(const std::shared_ptr<DictValue>& dict, EnvPtr env) const override;

    [[nodiscard]] Value eval(EnvPtr) const override;

    [[nodiscard]] QString toString() const override;
};
#endif //CPPYTHON_DICTPAIRNODE_H