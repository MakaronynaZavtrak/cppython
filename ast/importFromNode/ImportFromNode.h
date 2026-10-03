//
// Created by semyo on 03.10.2026.
//

#ifndef CPPYTHON_IMPORTFROMNODE_H
#define CPPYTHON_IMPORTFROMNODE_H
#include <QList>
#include <QPair>
#include "../ASTNode.h"

class ImportFromNode : public ASTNode {
public:
    QString moduleName;
    bool importAll = false;                        // from X import *
    QVector<QPair<QString, QString>> names;        // (имя, псевдоним)

    [[nodiscard]] Value eval(EnvPtr env) const override;
    [[nodiscard]] QString toString() const override;
    [[nodiscard]] bool shouldPrint() const override;
};
#endif //CPPYTHON_IMPORTFROMNODE_H