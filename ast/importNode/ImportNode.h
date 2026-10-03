//
// Created by semyo on 03.10.2026.
//

#ifndef CPPYTHON_IMPORTNODE_H
#define CPPYTHON_IMPORTNODE_H
#include <qlist.h>

#include "../ASTNode.h"


class ImportNode final : public ASTNode {
public:

    QVector<QPair<QString, QString>> imports;

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] bool shouldPrint() const override;
};


#endif //CPPYTHON_IMPORTNODE_H