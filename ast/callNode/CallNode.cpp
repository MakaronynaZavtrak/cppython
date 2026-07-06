//
// Created by semyo on 06.07.2026.
//

#include "CallNode.h"

#include "CallRuntime.h"

CallNode::CallNode(std::shared_ptr<ASTNode> callee,
                   std::vector<std::shared_ptr<ASTNode>> args,
                   std::vector<KeywordArg> kwargs)
        : callee(std::move(callee)),
        args(std::move(args)),
        kwargs(std::move(kwargs)) {}

[[nodiscard]] Value CallNode::eval(const EnvPtr env) const {
    const Value calleeVal = callee->eval(env);

    // positional
    std::vector<Value> evaluatedArgs;
    evaluatedArgs.reserve(args.size());

    for (const auto& arg : args)
        evaluatedArgs.push_back(arg->eval(env));

    // keyword
    std::vector<std::pair<QString, Value>> evaluatedKwargs;
    evaluatedKwargs.reserve(kwargs.size());

    for (const auto&[name, value] : kwargs) {

        evaluatedKwargs.emplace_back(
            name,
            value->eval(env)
        );
    }

    return call(calleeVal, evaluatedArgs, evaluatedKwargs, env);
}

[[nodiscard]] QString CallNode::toString() const {
    return callee->toString() + "(...)";
}