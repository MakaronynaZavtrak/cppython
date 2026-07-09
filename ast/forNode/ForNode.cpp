//
// Created by semyo on 06.07.2026.
//

#include "ForNode.h"

#include "CallRuntime.h"
#include "ClassUtils.h"
#include "Interpreter.h"
#include "../../exception/BreakException.h"
#include "../../exception/ContinueException.h"
#include "../../exception/StopIterationException.h"
#include "../../service/ExecutionHelpers.h"

ForNode::ForNode(QString varName,
                 std::shared_ptr<ASTNode> iterable,
                 std::vector<std::shared_ptr<ASTNode>> body)
        : varName(std::move(varName)),
          iterable(std::move(iterable)),
          body(std::move(body)) {}

Value ForNode::eval(EnvPtr env) const {

    Value iterableValue = iterable->eval(env);

    Value iterMethod = getAttrValue(iterableValue, "__iter__");

    Value iterator = call(iterMethod, {}, {}, env);

    Value last;

    while (true) {

        try {

            Value nextMethod = getAttrValue(iterator, "__next__");
            Value value = call(nextMethod, {}, {}, env);

            env->set(varName, value);

            try {

                for (const auto& stmt : body) {
                    last = Interpreter::executeNode(stmt, env);
                }

            }

            catch ([[maybe_unused]] const ContinueException& e) {}
            catch ([[maybe_unused]] const BreakException& e) {
                break;
            }

        }
        catch (const StopIterationException&) {
            break;
        }
    }

    return last;
}

QString ForNode::toString() const {
    return "for " + varName + " in " + iterable->toString() + ": ...";
}

bool ForNode::shouldPrint() const {
    return false;
}

bool ForNode::containsYield() const {

    return std::any_of(
        body.begin(),
        body.end(),
        [](const auto& stmt) { return stmt->containsYield(); });
}

Value ForNode::evalResumable(const EnvPtr env, ResumeContext& ctx) const {

    Value iterator;
    bool resuming = false;

    if (ctx.isReplaying()) {
        iterator = ctx.consumeReplayIterator();
        resuming = true;
    } else {
        Value iterableValue = iterable->eval(env);
        Value iterMethod = getAttrValue(iterableValue, "__iter__");
        iterator = call(iterMethod, {}, {}, env);
    }

    Value last;

    while (true) {

        if (!resuming) {

            Value value;

            try {
                Value nextMethod = getAttrValue(iterator, "__next__");
                value = call(nextMethod, {}, {}, env);
            }
            catch (const StopIterationException&) {
                break;
            }

            env->set(varName, value);
        }

        resuming = false;

        try {
            last = execBlockResumable(body, env, ctx);
        }
        catch (const YieldSignal&) {
            ctx.recordedIterators.insert(ctx.recordedIterators.begin(), iterator);
            throw;
        }
        catch ([[maybe_unused]] const ContinueException& e) {}
        catch ([[maybe_unused]] const BreakException& e) {
            break;
        }
    }

    return last;
}
