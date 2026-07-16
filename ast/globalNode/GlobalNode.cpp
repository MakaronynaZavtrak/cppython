//
// Created by semyo on 06.07.2026.
//

#include "GlobalNode.h"

#include "Environment.h"

Value GlobalNode::eval(const EnvPtr env) const {
    for (const auto& name : names) {
        env->globalVars.insert(name);
    }
    return {};
}

QString GlobalNode::toString() const {
    QString result = "GlobalNode(" + names.join(", ") + ")";
    return result;
}

bool GlobalNode::shouldPrint() const { return false; }