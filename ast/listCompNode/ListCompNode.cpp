//
// Created by semyo on 13.07.2026.
//

#include "ListCompNode.h"

#include "CallRuntime.h"
#include "ClassUtils.h"
#include "Environment.h"
#include "ListValue.h"
#include "../../exception/StopIterationException.h"
#include <functional>

Value ListCompNode::eval(const EnvPtr env) const {

    const auto scope = std::make_shared<Environment>(env);

    std::vector<Value> result;

    std::function<void(size_t)> run = [&](size_t idx) {

        if (idx == clauses.size()) {
            result.push_back(resultExpr->eval(scope));
            return;
        }

        const auto& clause = clauses[idx];

        // самый первый iterable вычисляется в ОКРУЖАЮЩЕМ scope,
        // остальные — уже во внутреннем (как в настоящем Python)
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
                if (!cond->eval(scope).toBool()) {
                    passed = false;
                    break;
                }
            }

            if (!passed) continue;

            run(idx + 1);
        }
    };

    run(0);

    return Value(std::make_shared<ListValue>(std::move(result)));
}

QString ListCompNode::toString() const {
    return "ListCompNode(...)";
}