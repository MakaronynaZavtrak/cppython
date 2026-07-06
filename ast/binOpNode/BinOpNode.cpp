//
// Created by semyo on 06.07.2026.
//

#include "BinOpNode.h"

#include <QHash>

#include "../../exception/SyntaxErrorException.h"

BinOpNode::BinOpNode(std::shared_ptr<ASTNode> left, QString  op, std::shared_ptr<ASTNode> right)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}

QString BinOpNode::toString() const {
    return "(" + left->toString() + " " + op + " " + right->toString() + ")";
}

Value BinOpNode::eval(const EnvPtr env) const {

    const Value l = left->eval(env);
    const Value r = right->eval(env);

    switch (parseOperation(op)) {
        case Operation::Add:            return l + r;
        case Operation::Subtract:       return l - r;
        case Operation::Multiply:       return l * r;
        case Operation::Power:          return l.power(r);
        case Operation::Divide:         return l / r;
        case Operation::Modulo:         return l % r;
        case Operation::IntDivide:      return l.intDivide(r);
        case Operation::BitOr:          return l | r;
        case Operation::BitAnd:         return l & r;
        case Operation::BitXor:         return l ^ r;

        default: throw SyntaxErrorException("Unsupported operation: " + op);
    }
}

BinOpNode::Operation BinOpNode::parseOperation(const QString &op) {
    static const QHash<QString, Operation> opMap = {
        {"+", Operation::Add},
        {"-", Operation::Subtract},
        {"*", Operation::Multiply},
        {"**", Operation::Power},
        {"/", Operation::Divide},
        {"%", Operation::Modulo},
        {"//", Operation::IntDivide},
        {"and", Operation::And},
        {"or", Operation::Or},
        {"|", Operation::BitOr},
        {"&", Operation::BitAnd},
        {"^", Operation::BitXor}
    };

    const auto it = opMap.find(op);

    if (it == opMap.end()) {
        throw SyntaxErrorException("Unsupported operation: " + op);
    }

    return it.value();
}