//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_DICTNODE_H
#define CPPYTHON_DICTNODE_H
#include "../dictElementNode/DictElementNode.h"

#include "../ASTNode.h"

class DictNode final : public ASTNode {

    std::vector<std::shared_ptr<DictElementNode>> items;

public:

    explicit DictNode(std::vector<std::shared_ptr<DictElementNode>> items);

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;
};
#endif //CPPYTHON_DICTNODE_H