//
// Created by semyo on 06.07.2026.
//

#include "NonlocalNode.h"

#include "Environment.h"

Value NonlocalNode::eval(const EnvPtr env) const {
    for (const auto& name : names) {
        env->nonlocalVars.insert(name);
    }
    return {};
}

QString NonlocalNode::toString() const {
    QString result = "NonlocalNode(" + names.join(", ") + ")";
    return result;
}

bool NonlocalNode::shouldPrint() const { return false; }