//
// Created by semyo on 06.07.2026.
//

#include "StarredNode.h"

StarredNode::StarredNode(std::shared_ptr<ASTNode> value)
    : value(std::move(value)) {}

 Value StarredNode::eval(const EnvPtr env) const {
    return value->eval(env);
}

QString StarredNode::toString() const {
    return "*" + value->toString();
}