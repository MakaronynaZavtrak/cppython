//
// Created by semyo on 06.07.2026.
//

#include "LogicalOpNode.h"

#include "../../exception/SyntaxErrorException.h"

LogicalOpNode::LogicalOpNode(std::shared_ptr<ASTNode> left, QString op,  std::shared_ptr<ASTNode> right) :
op(std::move(op)), left(std::move(left)), right(std::move(right)) {}

Value LogicalOpNode::eval(const EnvPtr env) const {

    Value l = left->eval(env);

    if (op == "and") {

        if (!l.toBool())
            return l;

        return right->eval(env);
    }

    if (op == "or") {

        if (l.toBool())
            return l;

        return right->eval(env);
    }

    throw SyntaxErrorException("Unknown logical operator");
}

QString LogicalOpNode::toString() const {
    return left->toString() + " " + op + " " + right->toString();
}