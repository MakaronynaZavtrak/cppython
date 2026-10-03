//
// Created by semyo on 03.10.2026.
//

#include "ImportNode.h"

#include "Environment.h"
#include "ModuleRegistry.h"
#include "../../exception/ModuleNotFoundErrorException.h"

Value ImportNode::eval(const EnvPtr env) const {

    for (const auto &[name, alias] : imports) {

        const Value::ModulePtr module = importBuiltinModule(name);

        if (!module) {
            throw ModuleNotFountErrorException("No module named '" + name + "'");
        }

        const QString bind = alias.isEmpty() ? name : alias;
        env->set(bind, Value(module));
    }

    return {};
}

QString ImportNode::toString() const {
    return "ImportNode";
}

bool ImportNode::shouldPrint() const {
    return false;
}
