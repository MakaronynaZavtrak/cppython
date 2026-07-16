//
// Created by semyo on 06.07.2026.
//

#include "CallNode.h"

#include "CallRuntime.h"
#include "DictValue.h"
#include "IteratorValue.h"
#include "StrValue.h"
#include "../../exception/PythonException.h"
#include "../../exception/StopIterationException.h"
#include "../starredNode/StarredNode.h"

CallNode::CallNode(std::shared_ptr<ASTNode> callee,
                   std::vector<std::shared_ptr<ASTNode>> args,
                   std::vector<KeywordArg> kwargs)
        : callee(std::move(callee)),
        args(std::move(args)),
        kwargs(std::move(kwargs)) {}

Value CallNode::eval(const EnvPtr env) const {

    const Value calleeVal = callee->eval(env);

    // positional
    std::vector<Value> evaluatedArgs;
    evaluatedArgs.reserve(args.size());

    for (const auto& arg : args) {

        if (const auto starred = dynamic_cast<StarredNode*>(arg.get())) {

            Value iterable = starred->value->eval(env);
            const auto iter = iterable.getIterator();

            while (true) {

                try {
                    evaluatedArgs.push_back(iter->next());
                }
                catch (const StopIterationException&) {
                    break;
                }
            }

            continue;
        }

        evaluatedArgs.push_back(arg->eval(env));
    }

    // keyword
    std::vector<std::pair<QString, Value>> evaluatedKwargs;
    evaluatedKwargs.reserve(kwargs.size());

    for (const auto& [name, value] : kwargs) {

        // пустое имя — это **expr
        if (name.isEmpty()) {

            Value dictVal = value->eval(env);
            const auto dict = dictVal.asDict("argument after **");

            for (const auto& key : dict->getOrder()) {

                if (!key.isString()) {
                    throw TypeErrorException("keywords must be strings");
                }

                evaluatedKwargs.emplace_back(
                    key.asString()->toString(),
                    dict->getItem(key)
                );
            }

            continue;
        }

        evaluatedKwargs.emplace_back(name, value->eval(env));
    }

    try {
        return call(calleeVal, evaluatedArgs, evaluatedKwargs, env);
    }
    catch (PythonException& e) {
        e.setPositionIfMissing(line, startColumn, endColumn, sourceId);
        e.recordFramePosition(startColumn, endColumn, calleeEndColumn, endColumn, true);
        e.captureTracebackIfMissing();
        throw;
    }
}

[[nodiscard]] QString CallNode::toString() const {
    return callee->toString() + "(...)";
}