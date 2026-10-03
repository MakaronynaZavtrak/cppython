//
// Created by semyo on 03.10.2026.
//

#include "ImportFromNode.h"
#include "Environment.h"
#include "ModuleRegistry.h"
#include "ModuleValue.h"
#include "../../exception/ImportErrorException.h"
#include "../../exception/ModuleNotFoundErrorException.h"

Value ImportFromNode::eval(const EnvPtr env) const {

    const Value::ModulePtr module = importBuiltinModule(moduleName);

    if (!module) {
        throw ModuleNotFountErrorException("No module named '" + moduleName + "'");
    }

    // from X import * — все имена, не начинающиеся с подчёркивания
    if (importAll) {
        for (auto it = module->members.begin(); it != module->members.end(); ++it) {
            if (!it.key().startsWith('_')) {
                env->set(it.key(), it.value());
            }
        }
        return {};
    }

    for (const auto &[name, alias] : names) {
        if (!module->members.contains(name)) {
            throw ImportErrorException(
                "cannot import name '" + name + "' from '" + moduleName + "'"
            );
        }
        const QString bind = alias.isEmpty() ? name : alias;
        env->set(bind, module->members[name]);
    }
    return {};
}

QString ImportFromNode::toString() const { return "ImportFromNode"; }

bool ImportFromNode::shouldPrint() const { return false; }