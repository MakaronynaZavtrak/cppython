//
// Created by semyo on 06.07.2026.
//

#include "CompareNode.h"

#include <QHash>

#include "../../exception/SyntaxErrorException.h"

CompareNode::Operation CompareNode::parseOperation(const QString &op) {
    static const QHash<QString, Operation> opMap = {
        {"==", Operation::Equal},
        {"!=", Operation::NotEqual},
        {"<", Operation::Less},
        {"<=", Operation::LessOrEqual},
        {">", Operation::Greater},
        {">=", Operation::GreaterOrEqual},
        {"in", Operation::In},
        {"not in", Operation::NotIn},
        {"is", Operation::Is},
        {"is not", Operation::IsNot},
    };

    const auto it = opMap.find(op);
    if (it == opMap.end()) {
        throw SyntaxErrorException("Unsupported operation: " + op);
    }
    return it.value();
}

bool CompareNode::compare(const Value& a,
                    const Value& b,
                    const Operation op) {

    switch (op) {
        case Operation::Equal:          return a == b;
        case Operation::NotEqual:       return a != b;
        case Operation::Less:           return a < b;
        case Operation::LessOrEqual:    return a <= b;
        case Operation::Greater:        return a > b;
        case Operation::GreaterOrEqual: return a >= b;
        case Operation::In:             return b.contains(a);
        case Operation::NotIn:          return !b.contains(a);
        case Operation::Is:             return a.is(b);
        case Operation::IsNot:          return !a.is(b);

        default: throw SyntaxErrorException(
            "Invalid comparison operation"
        );
    }
}

CompareNode::CompareNode(std::shared_ptr<ASTNode> lhs,
                std::vector<QString> operators,
                std::vector<std::shared_ptr<ASTNode>> rgs)
      : left(std::move(lhs)), ops(std::move(operators)), rights(std::move(rgs)) {}

Value CompareNode::eval(const EnvPtr env) const {

    Value a = left->eval(env);

    try {

        for (size_t i = 0; i < ops.size(); ++i) {

            Value b = rights[i]->eval(env);

            if (!compare(a, b, parseOperation(ops[i]))) {
                return Value(false);
            }

            a = std::move(b);
        }

        return Value(true);

    }
    catch (PythonException& e) {
        e.setPositionIfMissing(line, startColumn, endColumn, sourceId);
        e.recordFramePosition(startColumn, endColumn);
        e.captureTracebackIfMissing();
        throw;
    }
}

QString CompareNode::toString() const {

    QString out = "(" + left->toString() + ")";

    for (size_t i = 0; i < ops.size(); ++i) {
        out += " " + ops[i] + " " + rights[i]->toString();
    }

    return out;
}