//
// Created by semyo on 06.07.2026.
//

#include "TryNode.h"

#include "Environment.h"
#include "TupleValue.h"
#include "../../runtime/Runtime.h"
#include "../../service/ExecutionHelpers.h"

ExceptionScopeGuard::ExceptionScopeGuard(const Value::InstancePtr& instance) {
    Runtime::exceptionStack.push_back(instance);
}

ExceptionScopeGuard::~ExceptionScopeGuard() {
    Runtime::exceptionStack.pop_back();
}

TryNode::TryNode(
    std::vector<std::shared_ptr<ASTNode>> tryBody,
    std::vector<ExceptClause> excepts,
    std::vector<std::shared_ptr<ASTNode>> elseBody,
    std::vector<std::shared_ptr<ASTNode>> finallyBody)
    : tryBody(std::move(tryBody)),
      excepts(std::move(excepts)),
      elseBody(std::move(elseBody)),
      finallyBody(std::move(finallyBody)) {}

bool TryNode::matchesExceptionHandler(const Value &handlerValue, const Value::ClassPtr &excClass) {
    // except (A, B, C):
    if (handlerValue.isTuple()) {
        const auto tuple = handlerValue.asTuple();

        for (const auto &item: tuple->items) {
            if (!item.isClass()) {
                throw TypeErrorException(
                    "catching classes that do not inherit from BaseException is not allowed"
                );
            }

            if (const auto handlerClass = item.asClass();
                PythonException::isSubclass(excClass, handlerClass)) {
                return true;
            }
        }

        return false;
    }

    // except A:
    if (!handlerValue.isClass()) {
        throw TypeErrorException(
            "catching classes that do not inherit from BaseException is not allowed"
        );
    }

    const auto handlerClass = handlerValue.asClass();

    return PythonException::isSubclass(excClass, handlerClass);
}

Value TryNode::eval(const EnvPtr env) const {

    Value result;

    try {
        bool completedWithoutException = false;

        try {
            for (const auto &stmt: tryBody) {
                result = stmt->eval(env);
                printIfNeeded(stmt, result);
            }

            completedWithoutException = true;

        } catch (const PythonException &e) {
            bool handled = false;

            for (const auto &[exceptionExpr,
                     variableName,
                     body]: excepts) {

                if (!exceptionExpr) {
                    handled = true;

                } else {

                    if (Value value = exceptionExpr->eval(env);
                        !matchesExceptionHandler(value, e.getClass())) {
                        continue;
                    }

                    handled = true;
                }

                if (!variableName.isEmpty()) {
                    env->set(variableName, Value(e.getInstance()));
                }

                {
                    ExceptionScopeGuard guard(e.getInstance());

                    for (const auto &stmt: body) {
                        result = stmt->eval(env);
                        printIfNeeded(stmt, result);
                    }
                }

                break;
            }

            if (!handled) {
                throw;
            }
        }

        if (completedWithoutException) {
            for (const auto &stmt: elseBody) {
                result = stmt->eval(env);
                printIfNeeded(stmt, result);
            }
        }

    } catch (...) {

        for (const auto &stmt: finallyBody) {
            const Value finallyResult = stmt->eval(env);
            printIfNeeded(stmt, finallyResult);
        }

        throw;
    }

    for (const auto &stmt: finallyBody) {
        const Value finallyResult = stmt->eval(env);
        printIfNeeded(stmt, finallyResult);
    }

    return result;
}

QString TryNode::toString() const {

    QString result = "TryNode(\n";

    result += "try:\n";

    for (const auto &stmt: tryBody)
        result += "    " + stmt->toString() + "\n";

    for (const auto &[exceptionExpr, variableName, body]: excepts) {
        result += "except";

        if (exceptionExpr) {
            result += " ";
            result += exceptionExpr->toString();
        }

        if (!variableName.isEmpty()) {
            result += " as ";
            result += variableName;
        }

        result += ":\n";

        for (const auto &stmt: body)
            result += "    " + stmt->toString() + "\n";
    }

    if (!elseBody.empty()) {
        result += "else:\n";

        for (const auto &stmt: elseBody)
            result += "    " + stmt->toString() + "\n";
    }

    if (!finallyBody.empty()) {
        result += "finally:\n";

        for (const auto &stmt: finallyBody)
            result += "    " + stmt->toString() + "\n";
    }

    result += ")";

    return result;
}

bool TryNode::shouldPrint() const { return false; }

bool TryNode::containsYield() const {

    for (const auto& stmt : tryBody)
        if (stmt->containsYield())
            return true;

    for (const auto&[exceptionExpr, variableName, body]: excepts) {
        if (exceptionExpr && exceptionExpr->containsYield())
            return true;
    }

    for (const auto& stmt : elseBody)
        if (stmt->containsYield())
            return true;

    return std::any_of(
        finallyBody.begin(),
        finallyBody.end(),
        [](const auto& stmt) { return stmt->containsYield(); }
    );
}

Value TryNode::evalResumable(const EnvPtr env, ResumeContext& ctx) const {

    const size_t FINALLY_MARKER = excepts.size() + 2;

    Value result;
    std::exception_ptr pending = nullptr;

    if (ctx.isReplaying()) {

        const size_t region = ctx.consumeReplayStep();

        if (region == FINALLY_MARKER) {

            pending = ctx.consumeReplayPendingException();

        } else {

            try {
                result = runTryExceptElse(true, region, env, ctx);
            }
            catch (const YieldSignal&) {
                throw; // путь уже записан внутри runTryExceptElse
            }
            catch (...) {
                pending = std::current_exception();
            }
        }

    } else {

        try {
            result = runTryExceptElse(false, 0, env, ctx);
        }
        catch (const YieldSignal&) {
            throw;
        }
        catch (...) {
            pending = std::current_exception();
        }
    }

    try {
        execBlockResumable(finallyBody, env, ctx);
    }
    catch (const YieldSignal&) {
        ctx.recordedPath.insert(ctx.recordedPath.begin(), FINALLY_MARKER);
        ctx.recordedPendingExceptions.insert(ctx.recordedPendingExceptions.begin(), pending);
        throw;
    }

    if (pending) {
        std::rethrow_exception(pending);
    }

    return result;
}

Value TryNode::runTryExceptElse(bool resumingRegion, size_t region, const EnvPtr &env, ResumeContext& ctx) const {

    Value result;
    bool completedWithoutException = false;

    if (!resumingRegion || region == 0) {

        try {
            try {
                result = execBlockResumable(tryBody, env, ctx);
                completedWithoutException = true;
            }
            catch (const YieldSignal&) {
                ctx.recordedPath.insert(ctx.recordedPath.begin(), 0);
                throw;
            }
        }
        catch (const PythonException& e) {

            bool handled = false;

            for (size_t idx = 0; idx < excepts.size(); ++idx) {

                const auto& [exceptionExpr, variableName, body] = excepts[idx];

                if (!exceptionExpr) {
                    handled = true;
                } else {
                    Value value = exceptionExpr->eval(env);
                    if (!matchesExceptionHandler(value, e.getClass())) continue;
                    handled = true;
                }

                if (!variableName.isEmpty()) {
                    env->set(variableName, Value(e.getInstance()));
                }

                ExceptionScopeGuard guard(e.getInstance());

                try {
                    result = execBlockResumable(body, env, ctx);
                }
                catch (const YieldSignal&) {
                    ctx.recordedPath.insert(ctx.recordedPath.begin(), idx + 1);
                    ctx.recordedGuardInstances.insert(ctx.recordedGuardInstances.begin(), Value(e.getInstance()));
                    throw;
                }

                break;
            }

            if (!handled) throw;
        }

        if (completedWithoutException) {
            try {
                result = execBlockResumable(elseBody, env, ctx);
            }
            catch (const YieldSignal&) {
                ctx.recordedPath.insert(ctx.recordedPath.begin(), excepts.size() + 1);
                throw;
            }
        }

    } else if (region >= 1 && region <= excepts.size()) {

        size_t idx = region - 1;
        Value savedInstance = ctx.consumeReplayGuardInstance();
        const auto instance = savedInstance.asInstance();
        ExceptionScopeGuard guard(instance);

        try {
            result = execBlockResumable(excepts[idx].body, env, ctx);
        }
        catch (const YieldSignal&) {
            ctx.recordedPath.insert(ctx.recordedPath.begin(), idx + 1);
            ctx.recordedGuardInstances.insert(ctx.recordedGuardInstances.begin(), savedInstance);
            throw;
        }

    } else {
        try {
            result = execBlockResumable(elseBody, env, ctx);
        }
        catch (const YieldSignal&) {
            ctx.recordedPath.insert(ctx.recordedPath.begin(), excepts.size() + 1);
            throw;
        }
    }

    return result;
}
