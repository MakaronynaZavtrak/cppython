//
// Created by semyo on 06.07.2026.
//

#include "ValueNode.h"

ValueNode::ValueNode(Value value) : value(std::move(value)) {}

QString ValueNode::toString() const {
    return value.toString();
}

Value ValueNode::eval(EnvPtr env) const {
    return value;
}