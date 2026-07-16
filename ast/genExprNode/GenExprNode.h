//
// Created by semyo on 13.07.2026.
//

#ifndef CPPYTHON_GENEXPRNODE_H
#define CPPYTHON_GENEXPRNODE_H
#include "ComprehensionClause.h"
#include "../ASTNode.h"

class GenExprNode : public ASTNode {
    std::shared_ptr<ASTNode> resultExpr;
    std::vector<ComprehensionClause> clauses;

public:
    GenExprNode(std::shared_ptr<ASTNode> resultExpr, std::vector<ComprehensionClause> clauses)
        : resultExpr(std::move(resultExpr)), clauses(std::move(clauses)) {}

    [[nodiscard]] Value eval(EnvPtr env) const override;
    [[nodiscard]] QString toString() const override;
};
#endif