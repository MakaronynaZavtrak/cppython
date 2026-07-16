//
// Created by semyo on 09.07.2026.
//

#include "YieldFromNode.h"
#include "CallRuntime.h"
#include "ClassUtils.h"
#include "../../exception/AttributeErrorException.h"
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

    const bool resuming = ctx.isReplaying();
    Value iterator;

    if (resuming) {
        ctx.consumeReplayStep(); // маркер-заглушка
        iterator = ctx.consumeReplayIterator();
    } else {
        Value iterableValue = valueExpr->eval(env);
        Value iterMethod = getAttrValue(iterableValue, "__iter__");
        iterator = call(iterMethod, {}, {}, env);
    }

    Value value;
    bool gotStop = false;
    Value stopValue;

    try {

        if (resuming && ctx.hasPendingThrow) {

            bool hasThrow = true;
            Value throwMethod;

            try { throwMethod = getAttrValue(iterator, "throw"); }
            catch (const AttributeErrorException&) { hasThrow = false; }

            if (hasThrow) {
                value = call(throwMethod, { ctx.thrownInstance }, {}, env);
            } else {

                try {
                    Value closeMethod = getAttrValue(iterator, "close");
                    call(closeMethod, {}, {}, env);
                }
                catch (const AttributeErrorException&) {}

                throw PythonException(ctx.thrownInstance.asInstance());
            }

        } else if (resuming) {

            bool hasSend = true;
            Value sendMethod;

            try { sendMethod = getAttrValue(iterator, "send"); }
            catch (const AttributeErrorException&) { hasSend = false; }

            if (hasSend) {
                value = call(sendMethod, { ctx.sentValue }, {}, env);
            } else {
                Value nextMethod = getAttrValue(iterator, "__next__");
                value = call(nextMethod, {}, {}, env);
            }

        } else {
            Value nextMethod = getAttrValue(iterator, "__next__");
            value = call(nextMethod, {}, {}, env);
        }

    }
    catch (const StopIterationException& e) {
        gotStop = true;
        stopValue = extractStopIterationValue(e);
    }

    if (gotStop) {
        return stopValue;
    }

    try {
        throw YieldSignal(value);
    }
    catch (const YieldSignal&) {
        ctx.recordedIterators.insert(ctx.recordedIterators.begin(), iterator);
        ctx.recordedPath.insert(ctx.recordedPath.begin(), 0);
        throw;
    }
}

QString YieldFromNode::toString() const {
    return "YieldFromNode(" + valueExpr->toString() + ")";
}

bool YieldFromNode::isBareYieldStatement() const {
     return true;
}
