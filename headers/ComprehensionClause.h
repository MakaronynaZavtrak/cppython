//
// Created by semyo on 13.07.2026.
//

#ifndef CPPYTHON_COMPREHENSIONCLAUSE_H
#define CPPYTHON_COMPREHENSIONCLAUSE_H

#include <vector>
#include <memory>
#include <QString>

class ASTNode;

struct ComprehensionClause {
    QString varName;
    std::shared_ptr<ASTNode> iterable;
    std::vector<std::shared_ptr<ASTNode>> conditions;
};

#endif