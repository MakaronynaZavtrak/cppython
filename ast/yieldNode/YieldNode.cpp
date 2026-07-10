//
// Created by semyo on 07.07.2026.
//

#include "YieldNode.h"

#include "../../service/GeneratorControl.h"
#include "../../service/yieldSignal.h"

Value YieldNode::eval(const EnvPtr env) const {

    if (!GeneratorControl::pendingSendValues.empty()) {
        Value v = GeneratorControl::pendingSendValues.back();
        GeneratorControl::pendingSendValues.pop_back();
        return v;
    }

    const Value v = valueExpr ? valueExpr->eval(env) : Value();

    throw YieldSignal(v);
}

QString YieldNode::toString() const {
    return valueExpr
        ? "YieldNode(" + valueExpr->toString() + ")"
        : "YieldNode()";
}

bool YieldNode::isBareYieldStatement() const {
    return true;
}
