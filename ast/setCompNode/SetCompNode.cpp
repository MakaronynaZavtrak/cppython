//
// Created by semyo on 13.07.2026.
//

#include "SetCompNode.h"

#include "CallRuntime.h"
#include "ClassUtils.h"
#include "Environment.h"
#include "SetValue.h"
#include "../../exception/StopIterationException.h"
#include <functional>

Value SetCompNode::eval(const EnvPtr env) const {

    const auto scope = std::make_shared<Environment>(env);
    const auto setValue = std::make_shared<SetValue>();
    Value resultSet(setValue);

    std::function<void(size_t)> run = [&](size_t idx) {

        if (idx == clauses.size()) {
            Value item = resultExpr->eval(scope);
            Value addMethod = getAttrValue(resultSet, "add");
            call(addMethod, { item }, {}, scope);
            return;
        }

        const auto& clause = clauses[idx];

        Value iterableValue = clause.iterable->eval(idx == 0 ? env : scope);
        Value iterMethod = getAttrValue(iterableValue, "__iter__");
        Value iterator = call(iterMethod, {}, {}, scope);

        while (true) {

            Value item;

            try {
                Value nextMethod = getAttrValue(iterator, "__next__");
                item = call(nextMethod, {}, {}, scope);
            }
            catch (const StopIterationException&) {
                break;
            }

            scope->set(clause.varName, item);

            bool passed = true;
            for (const auto& cond : clause.conditions) {
                if (!cond->eval(scope).toBool()) { passed = false; break; }
            }

            if (!passed) continue;

            run(idx + 1);
        }
    };

    run(0);

    return resultSet;
}

QString SetCompNode::toString() const { return "SetCompNode(...)"; }