//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_DICTELEMENTNODE_H
#define CPPYTHON_DICTELEMENTNODE_H

#include "../ASTNode.h"

class DictElementNode : public ASTNode {
public:

    virtual void apply(
        const std::shared_ptr<DictValue>& dict,
        EnvPtr env) const = 0;
};
#endif //CPPYTHON_DICTELEMENTNODE_H