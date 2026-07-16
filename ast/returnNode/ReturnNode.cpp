//
// Created by semyo on 06.07.2026.
//

#include "ReturnNode.h"

#include "../../exception/ReturnException.h"

ReturnNode::ReturnNode(std::shared_ptr<ASTNode> expr) : expr(std::move(expr)) {}

Value ReturnNode::eval(const EnvPtr env) const {
    const Value val = expr ? expr->eval(env) : Value();
    throw ReturnException(val);
}

QString ReturnNode::toString() const {
    return "return ...";
}

bool ReturnNode::shouldPrint() const { return false; }
