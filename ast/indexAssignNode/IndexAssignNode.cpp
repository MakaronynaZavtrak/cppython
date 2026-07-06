//
// Created by semyo on 06.07.2026.
//

#include "IndexAssignNode.h"

#include "CallRuntime.h"
#include "ClassUtils.h"

IndexAssignNode::IndexAssignNode(std::shared_ptr<ASTNode> object,
                                 std::shared_ptr<ASTNode> index,
                                 std::shared_ptr<ASTNode> value)
        : object(std::move(object)),
          index(std::move(index)),
          value(std::move(value)) {}

Value IndexAssignNode::eval(EnvPtr env) const {

    Value obj = object->eval(env);
    Value idx = index->eval(env);
    Value val = value->eval(env);

    Value setitem = getAttrValue(obj, "__setitem__");

    call(setitem, {idx, val}, {}, env);

    return val;
}

QString IndexAssignNode::toString() const {
    return object->toString() + "[" + index->toString() + "] = " + value->toString();
}

bool IndexAssignNode::shouldPrint() const { return false; }