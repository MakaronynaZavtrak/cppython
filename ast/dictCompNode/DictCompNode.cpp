//
// Created by semyo on 13.07.2026.
//

#include "DictCompNode.h"

#include "CallRuntime.h"
#include "ClassUtils.h"
#include "Environment.h"
#include "DictValue.h"
#include "../../exception/StopIterationException.h"
#include <functional>

Value DictCompNode::eval(const EnvPtr env) const {

    const auto scope = std::make_shared<Environment>(env);
    const auto dictValue = std::make_shared<DictValue>();
    Value resultDict(dictValue);

    std::function<void(size_t)> run = [&](size_t idx) {

        if (idx == clauses.size()) {
            Value k = keyExpr->eval(scope);
            Value v = valueExpr->eval(scope);
            Value setitem = getAttrValue(resultDict, "__setitem__");
            call(setitem, { k, v }, {}, scope);
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

    return resultDict;
}

QString DictCompNode::toString() const { return "DictCompNode(...)"; }