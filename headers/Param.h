//
// Created by semyo on 27.04.2026.
//

#ifndef CPPYTHON_PARAM_H
#define CPPYTHON_PARAM_H
#include <QString>
class ASTNode;

struct Param {
    QString name;
    QString type;
    std::shared_ptr<ASTNode> defaultExpr = nullptr;
    bool isVarArgs = false;
};
#endif //CPPYTHON_PARAM_H