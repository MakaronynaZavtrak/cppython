//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_TUPLEASSIGNNODE_H
#define CPPYTHON_TUPLEASSIGNNODE_H

#include "../ASTNode.h"

class TupleAssignNode : public ASTNode {

public:
    std::vector<std::shared_ptr<ASTNode>> targets;
    std::shared_ptr<ASTNode> valueExpr;

    TupleAssignNode(std::vector<std::shared_ptr<ASTNode>> targets,
                    std::shared_ptr<ASTNode> valueExpr);

    static void assignSingle(const std::shared_ptr<ASTNode>& target,
                              const Value& value,
                              const EnvPtr& env);

    static void assignMultiple(const std::vector<std::shared_ptr<ASTNode>>& targets,
                                const Value& value,
                                const EnvPtr& env);

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] bool shouldPrint() const override;
};
#endif //CPPYTHON_TUPLEASSIGNNODE_H