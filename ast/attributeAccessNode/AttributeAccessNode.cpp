//
// Created by semyo on 06.07.2026.
//

#include "AttributeAccessNode.h"

#include "ClassUtils.h"

AttributeAccessNode::AttributeAccessNode(std::shared_ptr<ASTNode> object, QString attr)
        : object(std::move(object)), attr(std::move(attr)) {}

Value AttributeAccessNode::eval(const EnvPtr env) const {
    const Value objVal = object->eval(env);

    if (std::holds_alternative<Value::SuperPtr>(objVal.data)) {
        const auto super = std::get<Value::SuperPtr>(objVal.data);
        return getAttrFromSuper(super, attr);
    }

    return getAttrValue(objVal, attr);
}

QString AttributeAccessNode::toString() const {
    return object->toString() + "." + attr;
}