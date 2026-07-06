//
// Created by semyo on 06.07.2026.
//

#include "TupleAssignNode.h"

#include "CallRuntime.h"
#include "ClassUtils.h"
#include "Environment.h"
#include "IteratorValue.h"
#include "ListValue.h"
#include "../../exception/SyntaxErrorException.h"
#include "../../exception/ValueErrorException.h"
#include "../attributeAccessNode/AttributeAccessNode.h"
#include "../indexNode/IndexNode.h"
#include "../starredNode/StarredNode.h"
#include "../tupleNode/TupleNode.h"
#include "../varNode/VarNode.h"

TupleAssignNode::TupleAssignNode(std::vector<std::shared_ptr<ASTNode>> targets,
                                 std::shared_ptr<ASTNode> valueExpr)
        : targets(std::move(targets)), valueExpr(std::move(valueExpr)) {}

void TupleAssignNode::assignSingle(const std::shared_ptr<ASTNode>& target,
                          const Value& value,
                          const EnvPtr& env) {

    if (const auto var = std::dynamic_pointer_cast<VarNode>(target)) {
        env->set(var->name, value);
        return;
    }

    if (const auto attr = std::dynamic_pointer_cast<AttributeAccessNode>(target)) {
        Value objVal = attr->object->eval(env);
        setAttrValue(objVal, attr->attr, value);
        return;
    }

    if (const auto idx = std::dynamic_pointer_cast<IndexNode>(target)) {
        Value obj = idx->object->eval(env);
        Value index = idx->index->eval(env);
        Value setitem = getAttrValue(obj, "__setitem__");
        call(setitem, {index, value}, {}, env);
        return;
    }

    if (const auto nested = std::dynamic_pointer_cast<TupleNode>(target)) {
        assignMultiple(nested->elements, value, env);
        return;
    }

    throw SyntaxErrorException("Invalid assignment target in tuple unpacking");
}

void TupleAssignNode::assignMultiple(const std::vector<std::shared_ptr<ASTNode>>& targets,
                            const Value& value,
                            const EnvPtr& env) {

    std::vector<Value> values;
    const auto iter = value.getIterator();

    while (iter->hasNext()) {
        values.push_back(iter->next());
    }

    int starIndex = -1;

    for (size_t i = 0; i < targets.size(); ++i) {
        if (dynamic_cast<StarredNode*>(targets[i].get())) {

            if (starIndex != -1) {
                throw SyntaxErrorException(
                    "multiple starred expressions in assignment"
                );
            }
            starIndex = static_cast<int>(i);
        }
    }

    if (starIndex == -1) {

        if (values.size() != targets.size()) {

            if (values.size() < targets.size()) {
                throw ValueErrorException(
                    "not enough values to unpack (expected " +
                    QString::number(targets.size()) + ", got " +
                    QString::number(values.size()) + ")"
                );
            }

            throw ValueErrorException(
                "too many values to unpack (expected " +
                QString::number(targets.size()) + ", got " +
                QString::number(values.size()) + ")"
            );
        }

        for (size_t i = 0; i < targets.size(); ++i) {
            assignSingle(targets[i], values[i], env);
        }

    } else {

        const size_t before = starIndex;
        const size_t after = targets.size() - starIndex - 1;

        if (values.size() < before + after) {
            throw ValueErrorException("not enough values to unpack");
        }

        for (size_t i = 0; i < before; ++i) {
            assignSingle(targets[i], values[i], env);
        }

        std::vector middle(
            values.begin() + before,
            values.end() - after
        );

        const auto starred = dynamic_cast<StarredNode*>(targets[starIndex].get());
        assignSingle(starred->value, Value(std::make_shared<ListValue>(middle)), env);

        for (size_t i = 0; i < after; ++i) {
            assignSingle(
                targets[targets.size() - after + i],
                values[values.size() - after + i],
                env
            );
        }
    }
}

Value TupleAssignNode::eval(const EnvPtr env) const {
    Value rhs = valueExpr->eval(env);
    assignMultiple(targets, rhs, env);
    return rhs;
}

QString TupleAssignNode::toString() const {
    QStringList parts;
    for (const auto& t : targets) parts << t->toString();
    return parts.join(", ") + " = " + valueExpr->toString();
}

bool TupleAssignNode::shouldPrint() const { return false; }