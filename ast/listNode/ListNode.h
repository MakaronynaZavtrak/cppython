//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_LISTNODE_H
#define CPPYTHON_LISTNODE_H

#include "../ASTNode.h"

class ListNode : public ASTNode {
public:
    std::vector<std::shared_ptr<ASTNode>> elements;

    explicit ListNode(std::vector<std::shared_ptr<ASTNode>> elems);

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;
};
#endif //CPPYTHON_LISTNODE_H