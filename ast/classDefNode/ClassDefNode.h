//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_CLASSDEFNODE_H
#define CPPYTHON_CLASSDEFNODE_H

#include <qlist.h>
#include <utility>

#include "../ASTNode.h"

class ClassDefNode final : public ASTNode {
public:
    QString name;
    std::vector<std::shared_ptr<ASTNode>> baseExprs;
    QVector<std::shared_ptr<ASTNode>> body;
    std::vector<std::shared_ptr<ASTNode>> decorators;

    // keyword-аргументы заголовка класса: metaclass=... и прочие (name, expr).
    std::vector<std::pair<QString, std::shared_ptr<ASTNode>>> keywords;

    ClassDefNode(QString name,
                 std::vector<std::shared_ptr<ASTNode>> bases,
                 QVector<std::shared_ptr<ASTNode>> body,
                 std::vector<std::shared_ptr<ASTNode>> decorators = {},
                 std::vector<std::pair<QString, std::shared_ptr<ASTNode>>> keywords = {});

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] bool shouldPrint() const override;
};
#endif //CPPYTHON_CLASSDEFNODE_H