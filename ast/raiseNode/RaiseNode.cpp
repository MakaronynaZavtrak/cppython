//
// Created by semyo on 06.07.2026.
//

#include "RaiseNode.h"

#include "InstanceValue.h"
#include "../../exception/RuntimeErrorException.h"
#include "../../exception/TypeErrorException.h"

RaiseNode::RaiseNode(
        std::shared_ptr<ASTNode> exceptionExpr,
        std::shared_ptr<ASTNode> causeExpr)
    : exceptionExpr(std::move(exceptionExpr)),
      causeExpr(std::move(causeExpr)) {}

    void RaiseNode::raiseException(const Value& value, const Value& cause, bool hasCause) {

        if (!value.isInstance()) {
            throw TypeErrorException(
                "exceptions must derive from BaseException"
            );
        }

        const auto instance = value.asInstance();

        if (!PythonException::isSubclass(
                instance->klass,
                Runtime::baseExceptionClass)) {
            throw TypeErrorException(
                "exceptions must derive from BaseException"
            );
        }

        if (hasCause) {
            instance->fields["__cause__"] = cause;
            instance->fields["__suppress_context__"] = Value(true);
        }

        throw PythonException(instance);
    }

    Value RaiseNode::eval(const EnvPtr env) const {

        if (!exceptionExpr) {

            if (Runtime::exceptionStack.empty()) {
                throw RuntimeErrorException(
                    "No active exception to re-raise"
                );
            }

            const auto& instance = Runtime::exceptionStack.back();

            throw PythonException(instance);
        }

        const Value value = exceptionExpr->eval(env);

        Value cause;
        bool hasCause = false;

        if (causeExpr) {

            hasCause = true;
            cause = causeExpr->eval(env);

            // raise X from None — явное подавление chaining
            if (!cause.isNone() && !cause.isInstance()) {
                throw TypeErrorException(
                    "exception causes must derive from BaseException"
                );
            }

            if (cause.isInstance() &&
                !PythonException::isSubclass(
                    cause.asInstance()->klass,
                    Runtime::baseExceptionClass)) {
                throw TypeErrorException(
                    "exception causes must derive from BaseException"
                );
            }
        }

        // raise Exception(...)
        if (value.isInstance()) {
            raiseException(value, cause, hasCause);
        }

        // raise Exception
        if (value.isClass()) {

            const auto klass = value.asClass();

            if (!PythonException::isSubclass(
                    klass,
                    Runtime::baseExceptionClass)) {
                throw TypeErrorException(
                    "exceptions must derive from BaseException"
                );
            }

            const auto instance =
                    PythonException::makeInstance(
                        klass,
                        ""
                    );

            if (hasCause) {
                instance->fields["__cause__"] = cause;
                instance->fields["__suppress_context__"] = Value(true);
            }

            throw PythonException(instance);
        }

        // raise 123
        throw TypeErrorException(
            "exceptions must derive from BaseException"
        );
    }

    QString RaiseNode::toString() const {

        if (!exceptionExpr) {
            return "RaiseNode()";
        }

        if (causeExpr) {
            return QString("RaiseNode(%1 from %2)")
                .arg(exceptionExpr->toString(), causeExpr->toString());
        }

        return QString("RaiseNode(%1)")
            .arg(exceptionExpr->toString());
    }