//
// Created by semyo on 06.07.2026.
//

#include "WhileNode.h"

#include "Interpreter.h"
#include "../../exception/BreakException.h"
#include "../../exception/ContinueException.h"
#include "../../service/execBlockResumable.h"

WhileNode::WhileNode(std::shared_ptr<ASTNode> condition,
                     std::vector<std::shared_ptr<ASTNode>> body,
                     std::vector<std::shared_ptr<ASTNode>> elseBody)
            : condition(std::move(condition)),
            body(std::move(body)),
            elseBody(std::move(elseBody)) {}

Value WhileNode::eval(const EnvPtr env) const {

    Value last;
    bool broken = false;

    while (condition->eval(env).toBool()) {

        try {
            for (auto& stmt : body) {
                last = Interpreter::executeNode(stmt, env);
            }
        }
        catch ([[maybe_unused]] const ContinueException& e) {}
        catch ([[maybe_unused]] const BreakException& e) {
            broken = true;
            break;
        }
    }

    if (!broken) {
        for (auto& stmt : elseBody) {
            last = Interpreter::executeNode(stmt, env);
        }
    }
    return last;
}

QString WhileNode::toString() const {

    QString out = "while " + condition->toString() + ":\n";

    for (auto& stmt : body) {
        out += "\t" + stmt->toString() + "\n";
    }

    if (!elseBody.empty()) {

        out += "else:\n";

        for (auto& stmt : elseBody) {
            out += "\t" + stmt->toString() + "\n";
        }
    }

    return out;
}

bool WhileNode::shouldPrint() const { return false; }

bool WhileNode::containsYield() const {

    if (condition->containsYield())
        return true;

    for (const auto& stmt : body)
        if (stmt->containsYield())
            return true;

    return std::any_of(
        elseBody.begin(),
        elseBody.end(),
        [](const auto& stmt) { return stmt->containsYield(); }
    );
}

Value WhileNode::evalResumable(const EnvPtr env, ResumeContext& ctx) const {

    size_t skipUntil = 0;
    bool resumingElse = false;

    if (ctx.isReplaying()) {

        size_t region = ctx.consumeReplayStep();

        if (region == 1) {
            resumingElse = true;
        } else {
            skipUntil = ctx.consumeReplayStep();
        }
    }

    Value last;
    bool broken = false;

    if (!resumingElse) {

        size_t iteration = 0;

        while (true) {

            bool condTrue;

            if (iteration < skipUntil) {
                // уже реально пройдено в прошлых next() — доверяем, что было true
                condTrue = true;
            } else {
                condTrue = condition->eval(env).toBool();
            }

            if (!condTrue) break;

            try {
                last = execBlockResumable(body, env, ctx);
            }
            catch (const YieldSignal&) {
                ctx.recordedPath.insert(ctx.recordedPath.begin(), iteration);
                ctx.recordedPath.insert(ctx.recordedPath.begin(), 0);
                throw;
            }
            catch (const ContinueException&) {
                iteration++;
                continue;
            }
            catch (const BreakException&) {
                broken = true;
                break;
            }

            iteration++;
        }
    }

    if (!broken) {
        try {
            last = execBlockResumable(elseBody, env, ctx);
        }
        catch (const YieldSignal&) {
            ctx.recordedPath.insert(ctx.recordedPath.begin(), 1);
            throw;
        }
    }

    return last;
}
