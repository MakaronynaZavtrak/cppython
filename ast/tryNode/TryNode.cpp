//
// Created by semyo on 06.07.2026.
//

#include "TryNode.h"

#include "Environment.h"
#include "TupleValue.h"
#include "../../runtime/Runtime.h"

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

    for (const auto& stmt : finallyBody)
        if (stmt->containsYield())
            return true;

    return false;
}