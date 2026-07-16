//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_GLOBALNODE_H
#define CPPYTHON_GLOBALNODE_H

#include <qlist.h>

#include "../ASTNode.h"

class GlobalNode : public ASTNode {
public:

    QVector<QString> names;

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] bool shouldPrint() const override;
};
#endif //CPPYTHON_GLOBALNODE_H