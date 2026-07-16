//
// Created by semyo on 06.07.2026.
//

#include "AssignNode.h"

#include "Environment.h"
#include "../../service/yieldSignal.h"
#include "../yieldFromNode/YieldFromNode.h"

AssignNode::AssignNode(QString varName, std::shared_ptr<ASTNode> valueExpr) :
    varName(std::move(varName)), valueExpr(std::move(valueExpr)) {}

Value AssignNode::eval(const EnvPtr env) const {
    Value val = valueExpr->eval(env);
    env.get()->set(varName, val);
    return val;
}

QString AssignNode::toString() const {
    return varName + " = " + valueExpr->toString();
}

bool AssignNode::containsYield() const {
    return valueExpr->containsYield();
}

bool AssignNode::shouldPrint() const { return false; }

Value AssignNode::evalResumable(const EnvPtr env, ResumeContext& ctx) const {

    if (dynamic_cast<YieldFromNode*>(valueExpr.get())) {

        Value val;

        try {
            val = valueExpr->evalResumable(env, ctx);
        }
        catch (const YieldSignal&) {
            ctx.recordedPath.insert(ctx.recordedPath.begin(), 0);
            throw;
        }

        env->set(varName, val);
        return val;
    }

    Value val = valueExpr->eval(env);
    env->set(varName, val);
    return val;
}
