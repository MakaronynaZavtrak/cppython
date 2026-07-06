//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_AUGASSIGNNODE_H
#define CPPYTHON_AUGASSIGNNODE_H

#include "../ASTNode.h"

class AugAssignNode : public ASTNode {

    enum class Operation {
        Add,
        Subtract,
        Multiply,
        Divide,
        IntDivide,
        Modulo,
        Power,
        BitOr,
        BitAnd,
        BitXor
    };

    QString name;
    QString op;
    std::shared_ptr<ASTNode> value;

    static Operation parseOperation(const QString& op);

    static Value tryInplaceOperation(
    const Value& left,
    const QString& methodName,
    const Value& right,
    const EnvPtr& env,
    const std::function<Value()>& fallback);

public:

    AugAssignNode(QString name, QString op, std::shared_ptr<ASTNode> value);

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] bool shouldPrint() const override;
};
#endif //CPPYTHON_AUGASSIGNNODE_H