//
// Created by semyo on 06.07.2026.
//

#include "ContinueNode.h"

#include "../../exception/ContinueException.h"

Value ContinueNode::eval(EnvPtr env) const { throw ContinueException(); }

QString ContinueNode::toString() const { return "continue"; }