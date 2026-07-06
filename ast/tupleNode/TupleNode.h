//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_TUPLENODE_H
#define CPPYTHON_TUPLENODE_H

#include "../ASTNode.h"

class TupleNode : public ASTNode {
public:
    std::vector<std::shared_ptr<ASTNode>> elements;

    explicit TupleNode(std::vector<std::shared_ptr<ASTNode>> elements);

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;
};
#endif //CPPYTHON_TUPLENODE_H