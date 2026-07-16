//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_UNARYOPNODE_H
#define CPPYTHON_UNARYOPNODE_H

#include "../ASTNode.h"

class UnaryOpNode final : public ASTNode {

    QString op;
    std::shared_ptr<ASTNode> operand;

    enum class Operation {
        Not,
        UnaryPlus,
        UnaryMinus
    };

    static Operation parseOperation(const QString &op);

public:

    UnaryOpNode(QString  op, std::shared_ptr<ASTNode> operand);

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;
};
#endif //CPPYTHON_UNARYOPNODE_H