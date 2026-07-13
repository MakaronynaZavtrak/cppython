//
// Created by semyo on 13.07.2026.
//

#ifndef CPPYTHON_LISTCOMPNODE_H
#define CPPYTHON_LISTCOMPNODE_H

#include "ComprehensionClause.h"
#include "../ASTNode.h"

class ListCompNode : public ASTNode {
    std::shared_ptr<ASTNode> resultExpr;
    std::vector<ComprehensionClause> clauses;

public:
    ListCompNode(std::shared_ptr<ASTNode> resultExpr, std::vector<ComprehensionClause> clauses)
        : resultExpr(std::move(resultExpr)), clauses(std::move(clauses)) {}

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;
};

#endif