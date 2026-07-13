//
// Created by semyo on 13.07.2026.
//

#ifndef CPPYTHON_DICTCOMPNODE_H
#define CPPYTHON_DICTCOMPNODE_H
#include "ComprehensionClause.h"
#include "../ASTNode.h"

class DictCompNode : public ASTNode {
    std::shared_ptr<ASTNode> keyExpr;
    std::shared_ptr<ASTNode> valueExpr;
    std::vector<ComprehensionClause> clauses;

public:
    DictCompNode(std::shared_ptr<ASTNode> keyExpr,
                std::shared_ptr<ASTNode> valueExpr,
                std::vector<ComprehensionClause> clauses)
        : keyExpr(std::move(keyExpr)), valueExpr(std::move(valueExpr)), clauses(std::move(clauses)) {}

    [[nodiscard]] Value eval(EnvPtr env) const override;
    [[nodiscard]] QString toString() const override;
};
#endif