//
// Created by semyo on 06.07.2026.
//

#include "IfNode.h"

#include "Interpreter.h"
#include "../../service/execBlockResumable.h"

IfNode::IfNode(std::shared_ptr<ASTNode> condition,
               std::vector<std::shared_ptr<ASTNode>> body,
               std::vector<std::pair<std::shared_ptr<ASTNode>, std::vector<std::shared_ptr<ASTNode>>>> elifs,
               std::vector<std::shared_ptr<ASTNode>> elseBody)
    : condition(std::move(std::move(condition))), body(std::move(body)), elifs(std::move(elifs)), elseBody(std::move(elseBody)) {}

    Value IfNode::eval(EnvPtr env) const {

        if (condition->eval(env).toBool()) {

            Value lastValue;

            for (const auto& stmt : body) {
                lastValue = Interpreter::executeNode(stmt, env);
            }

            return lastValue;
        }

        for (const auto&[fst, snd]: elifs) {

            if (fst->eval(env).toBool()) {

                Value lastValue;

                for (const auto& stmt : snd) {
                    lastValue = Interpreter::executeNode(stmt, env);
                }

                return lastValue;
            }
        }

        if (!elseBody.empty()) {

            Value lastValue;

            for (const auto& stmt : elseBody) {
                lastValue = Interpreter::executeNode(stmt, env);
            }

            return lastValue;
        }

        return {};
    }

    QString IfNode::toString() const {

        QString result = "if " + condition->toString() + ":\n";

        for (const auto& stmt : body) {
            result += "    " + stmt->toString() + "\n";
        }

        for (const auto&[fst, snd] : elifs) {

            result += "elif " + fst->toString() + ":\n";

            for (const auto& stmt : snd) {
                result += "    " + stmt->toString() + "\n";
            }
        }

        if (!elseBody.empty()) {

            result += "else:\n";

            for (const auto& stmt : elseBody) {
                result += "    " + stmt->toString() + "\n";
            }
        }

        return result;
    }

    bool IfNode::shouldPrint() const { return false; }

bool IfNode::containsYield() const {

    if (condition->containsYield()) {
        return true;
    }

    for (const auto& stmt : body) {
        if (stmt->containsYield()) {
            return true;
        }
    }

    for (const auto&[fst, snd] : elifs) {

        if (fst->containsYield()) {
            return true;
        }

        if (std::any_of(
            snd.begin(),
            snd.end(),
            [](const auto& sub_stmt) { return sub_stmt->containsYield(); })) {
            return true;
        }
    }

    for (const auto& stmt : elseBody)
        if (stmt->containsYield())
            return true;

    return false;
}

Value IfNode::evalResumable(const EnvPtr env, ResumeContext& ctx) const {

    if (ctx.isReplaying()) {

        const size_t branch = ctx.consumeReplayStep();

        try {

            if (branch == 0) {
                return execBlockResumable(body, env, ctx);
            }

            if (branch <= elifs.size()) {
                return execBlockResumable(elifs[branch - 1].second, env, ctx);
            }

            return execBlockResumable(elseBody, env, ctx);

        }
        catch (const YieldSignal&) {
            ctx.recordedPath.insert(ctx.recordedPath.begin(), branch);
            throw;
        }
    }

    // обычное прямое выполнение (не resume) — как в eval(), но с записью пути при yield
    if (condition->eval(env).toBool()) {
        try {
            return execBlockResumable(body, env, ctx);
        }
        catch (const YieldSignal&) {
            ctx.recordedPath.insert(ctx.recordedPath.begin(), 0);
            throw;
        }
    }

    for (size_t i = 0; i < elifs.size(); ++i) {

        if (elifs[i].first->eval(env).toBool()) {
            try {
                return execBlockResumable(elifs[i].second, env, ctx);
            }
            catch (const YieldSignal&) {
                ctx.recordedPath.insert(ctx.recordedPath.begin(), i + 1);
                throw;
            }
        }
    }

    if (!elseBody.empty()) {
        try {
            return execBlockResumable(elseBody, env, ctx);
        }
        catch (const YieldSignal&) {
            ctx.recordedPath.insert(ctx.recordedPath.begin(), elifs.size() + 1);
            throw;
        }
    }

    return {};
}
