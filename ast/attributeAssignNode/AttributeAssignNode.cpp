//
// Created by semyo on 06.07.2026.
//

#include "AttributeAssignNode.h"

#include "ClassUtils.h"

AttributeAssignNode::AttributeAssignNode(std::shared_ptr<ASTNode> object,
                                         QString attr,
                                         std::shared_ptr<ASTNode> valueExpr)
        : object(std::move(object)),
          attr(std::move(attr)),
          valueExpr(std::move(valueExpr)) {}

Value AttributeAssignNode::eval(const EnvPtr env) const {
    Value objVal = object->eval(env);
    Value val = valueExpr->eval(env);

    setAttrValue(objVal, attr, val);

    return val;
}

QString AttributeAssignNode::toString() const {
    return object->toString() + "." + attr + " = " + valueExpr->toString();
}

bool AttributeAssignNode::shouldPrint() const { return false; }