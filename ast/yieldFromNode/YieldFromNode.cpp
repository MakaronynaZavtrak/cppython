//
// Created by semyo on 09.07.2026.
//

#include "YieldFromNode.h"
#include "CallRuntime.h"
#include "ClassUtils.h"
#include "../../exception/StopIterationException.h"
#include "../../service/ExecutionHelpers.h"

Value YieldFromNode::eval(const EnvPtr env) const {
    // без возобновляемого контекста (не внутри генератора) — не имеет смысла,
    // но чтобы не падать молча, прогоняем как обычный "простой" цикл до конца
    Value iterableValue = valueExpr->eval(env);
    Value iterMethod = getAttrValue(iterableValue, "__iter__");
    Value iterator = call(iterMethod, {}, {}, env);

    while (true) {
        try {
            Value nextMethod = getAttrValue(iterator, "__next__");
            Value value = call(nextMethod, {}, {}, env);
            throw YieldSignal(value);
        }
        catch (const StopIterationException& e) {
            return extractStopIterationValue(e);
        }
    }
}

Value YieldFromNode::evalResumable(const EnvPtr env, ResumeContext& ctx) const {

    Value iterator;

    if (ctx.isReplaying()) {
        ctx.consumeReplayStep(); // маркер-заглушка, значение не важно
        iterator = ctx.consumeReplayIterator();
    } else {
        Value iterableValue = valueExpr->eval(env);
        Value iterMethod = getAttrValue(iterableValue, "__iter__");
        iterator = call(iterMethod, {}, {}, env);
    }

    while (true) {

        Value value;

        try {
            Value nextMethod = getAttrValue(iterator, "__next__");
            value = call(nextMethod, {}, {}, env);
        }
        catch (const StopIterationException& e) {
            return extractStopIterationValue(e);
        }

        try {
            throw YieldSignal(value);
        }
        catch (const YieldSignal&) {
            ctx.recordedIterators.insert(ctx.recordedIterators.begin(), iterator);
            ctx.recordedPath.insert(ctx.recordedPath.begin(), 0); // маркер "ещё не закончили"
            throw;
        }
    }
}

QString YieldFromNode::toString() const {
    return "YieldFromNode(" + valueExpr->toString() + ")";
}

bool YieldFromNode::isBareYieldStatement() const {
     return true;
}
