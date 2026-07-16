//
// Created by semyo on 06.07.2026.
//

#include "AugAssignNode.h"

#include "CallRuntime.h"
#include "ClassUtils.h"
#include "../../exception/AttributeErrorException.h"
#include "../../exception/SyntaxErrorException.h"

AugAssignNode::Operation AugAssignNode::parseOperation(const QString& op) {

    static const std::unordered_map<QString, Operation> opMap = {
        {"+=",  Operation::Add},
        {"-=",  Operation::Subtract},
        {"*=",  Operation::Multiply},
        {"/=",  Operation::Divide},
        {"//=", Operation::IntDivide},
        {"%=",  Operation::Modulo},
        {"**=", Operation::Power},
        {"|=", Operation::BitOr},
        {"&=", Operation::BitAnd},
        {"^=", Operation::BitXor}
    };

    const auto it = opMap.find(op);

    if (it == opMap.end()) {
        throw SyntaxErrorException(
            "Unsupported augmented assignment: " + op
        );
    }

    return it->second;
}

Value AugAssignNode::tryInplaceOperation(
    const Value& left,
    const QString& methodName,
    const Value& right,
    const EnvPtr& env,
    const std::function<Value()>& fallback) {

    try {

        const Value method = getAttrValue(left, methodName);

        return call(method, { right }, {}, env);

    } catch ([[maybe_unused]] const AttributeErrorException& e) {}

    return fallback();
}

AugAssignNode::AugAssignNode(QString name, QString op, std::shared_ptr<ASTNode> value)
        : name(std::move(name)), op(std::move(op)), value(std::move(value)) {}

Value AugAssignNode::eval(EnvPtr env) const {

    Value left = env->get(name);
    Value right = value->eval(env);

    Value result;

    switch (parseOperation(op)) {
        case Operation::Add:
            result = tryInplaceOperation(
                left,
                "__iadd__",
                right,
                env,
                [&] { return left + right; }
            );
            break;

        case Operation::Subtract:
            result = tryInplaceOperation(
                left,
                "__isub__",
                right,
                env,
                [&] { return left - right; }
            );
            break;

        case Operation::Multiply:
            result = tryInplaceOperation(
                left,
                "__imul__",
                right,
                env,
                [&] { return left * right; }
            );
            break;

        case Operation::Divide:
            result = tryInplaceOperation(
                left,
                "__itruediv__",
                right,
                env,
                [&] { return left / right; }
            );
            break;

        case Operation::IntDivide:
            result = tryInplaceOperation(
                left,
                "__ifloordiv__",
                right,
                env,
                [&] { return left.intDivide(right); }
            );
            break;

        case Operation::Modulo:
            result = tryInplaceOperation(
                left,
                "__imod__",
                right,
                env,
                [&] { return left % right; }
            );
            break;

        case Operation::Power:
            result = tryInplaceOperation(
                left,
                "__ipow__",
                right,
                env,
                [&] { return left.power(right); }
            );
            break;

        case Operation::BitOr:
            result = tryInplaceOperation(
                left,
                "__ior__",
                right,
                env,
                [&] { return left | right; }
            );
            break;

        case Operation::BitAnd:
            result = tryInplaceOperation(
                left,
                "__iand__",
                right,
                env,
                [&] { return left & right; }
            );
            break;

        case Operation::BitXor:
            result = tryInplaceOperation(
                left,
                "__ixor__",
                right,
                env,
                [&] { return left ^ right; }
            );
            break;

        default:
            throw SyntaxErrorException("Unsupported augmented assignment");
    }

    env->set(name, result);

    return result;
}

QString AugAssignNode::toString() const {
    return name + " " + op + " " + value->toString();
}

bool AugAssignNode::shouldPrint() const { return false; }