//
// Created by semyo on 06.07.2026.
//

#include "BreakNode.h"

#include "../../exception/BreakException.h"

Value BreakNode::eval(EnvPtr env) const { throw BreakException(); }

QString BreakNode::toString() const { return "break"; }
