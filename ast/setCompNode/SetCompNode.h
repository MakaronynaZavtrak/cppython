//
// Created by semyo on 13.07.2026.
//

#ifndef CPPYTHON_SETCOMPNODE_H
#define CPPYTHON_SETCOMPNODE_H
#include "ComprehensionClause.h"
#include "../ASTNode.h"

class SetCompNode : public ASTNode {
    std::shared_ptr<ASTNode> resultExpr;
    std::vector<ComprehensionClause> clauses;

public:
    SetCompNode(std::shared_ptr<ASTNode> resultExpr, std::vector<ComprehensionClause> clauses)
        : resultExpr(std::move(resultExpr)), clauses(std::move(clauses)) {}

    [[nodiscard]] Value eval(EnvPtr env) const override;
    [[nodiscard]] QString toString() const override;
};
#endif