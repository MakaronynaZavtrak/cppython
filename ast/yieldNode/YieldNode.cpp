//
// Created by semyo on 07.07.2026.
//

#include "YieldNode.h"

#include "../../service/yieldSignal.h"

Value YieldNode::eval(const EnvPtr env) const {
    const Value v = valueExpr ? valueExpr->eval(env) : Value();
    throw YieldSignal(v);
}

QString YieldNode::toString() const {
    return valueExpr
        ? "YieldNode(" + valueExpr->toString() + ")"
        : "YieldNode()";
}