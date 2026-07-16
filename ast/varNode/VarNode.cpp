//
// Created by semyo on 06.07.2026.
//

#include "VarNode.h"

#include "Environment.h"

VarNode::VarNode(QString name) : name(std::move(name)) {}

QString VarNode::toString() const { return name; }

Value VarNode::eval(const EnvPtr env) const { return env.get()->get(name); }