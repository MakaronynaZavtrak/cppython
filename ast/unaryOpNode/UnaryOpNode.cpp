//
// Created by semyo on 06.07.2026.
//

#include "UnaryOpNode.h"

#include <QHash>

#include "../../exception/SyntaxErrorException.h"

UnaryOpNode::Operation UnaryOpNode::parseOperation(const QString &op) {

    static const QHash<QString, Operation> opMap = {
        {"+", Operation::UnaryPlus},
        {"-", Operation::UnaryMinus},
        {"not", Operation::Not}
    };

    const auto it = opMap.find(op);
    if (it == opMap.end()) {
        throw SyntaxErrorException("Unsupported operation: " + op);
    }
    return it.value();
}

UnaryOpNode::UnaryOpNode(QString  op, std::shared_ptr<ASTNode> operand)
        : op(std::move(op)), operand(std::move(operand)) {}

Value UnaryOpNode::eval(const EnvPtr env) const {

    const Value val = operand->eval(env);

    switch (parseOperation(op)) {
        case Operation::Not:        return Value(!val.toBool());
        case Operation::UnaryPlus:  return +val;
        case Operation::UnaryMinus: return -val;

        default: throw SyntaxErrorException("Unsupported operation: " + op);
    }


}

QString UnaryOpNode::toString() const {
    return "(" + operand->toString() + op + ")";
}