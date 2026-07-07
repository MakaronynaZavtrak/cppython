//
// Created by semyo on 06.07.2026.
//

#include "FunctionDefNode.h"

#include "CallRuntime.h"
#include "FunctionValue.h"

FunctionDefNode::FunctionDefNode(QString name,
                                 std::vector<Param> params,
                                 std::vector<std::shared_ptr<ASTNode>> body,
                                 std::vector<std::shared_ptr<ASTNode>> decorators)
    : name(std::move(name)),
    params(std::move(params)),
    body(std::move(body)),
    decorators(std::move(decorators)) {}

Value FunctionDefNode::eval(const EnvPtr env) const {

    const auto func = std::make_shared<FunctionValue>(params, body, env, name);

    for (const auto& stmt : body) {
        if (stmt->containsYield()) {
            func->isGenerator = true;
            break;
        }
    }

    Value v(func);

    // применяем декораторы снизу вверх
    for (auto it = decorators.rbegin(); it != decorators.rend(); ++it) {
        Value decorator = (*it)->eval(env);
        v = call(decorator, { v }, {}, env);
    }

    env->set(name, v);

    return v;
}

QString FunctionDefNode::toString() const {

    QString out = "def " + name + "(";

    for (size_t i = 0; i < params.size(); ++i) {
        out += params[i].name;

        if (!params[i].type.isEmpty()) {
            out += ": " + params[i].type;
        }

        if (i + 1 < params.size())
            out += ", ";
    }
    out += "):\n";

    for (auto& stmt : body) {
        out += "\t" + stmt->toString() + "\n";
    }

    return out;
}

bool FunctionDefNode::shouldPrint() const { return false; }