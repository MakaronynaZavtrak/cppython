//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_SLICENODE_H
#define CPPYTHON_SLICENODE_H

#include "../ASTNode.h"

class SliceNode : public ASTNode {
public:

    std::shared_ptr<ASTNode> start;
    std::shared_ptr<ASTNode> stop;
    std::shared_ptr<ASTNode> step;

    SliceNode(
        std::shared_ptr<ASTNode> start,
        std::shared_ptr<ASTNode> stop,
        std::shared_ptr<ASTNode> step);


    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;
};
#endif //CPPYTHON_SLICENODE_H