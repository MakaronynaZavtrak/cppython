//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_DICTUNPACKNODE_H
#define CPPYTHON_DICTUNPACKNODE_H

#include "../DictElementNode.h"

class DictUnpackNode : public DictElementNode {
public:
    std::shared_ptr<ASTNode> value;

    explicit DictUnpackNode(std::shared_ptr<ASTNode> value);

    void apply(const std::shared_ptr<DictValue>& dict, const EnvPtr env) const override;

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;
};
#endif //CPPYTHON_DICTUNPACKNODE_H