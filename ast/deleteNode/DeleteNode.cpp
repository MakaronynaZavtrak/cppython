//
// Created by semyo on 06.07.2026.
//

#include "DeleteNode.h"

#include "CallRuntime.h"
#include "ClassUtils.h"
#include "../../exception/SyntaxErrorException.h"
#include "../indexNode/IndexNode.h"

DeleteNode::DeleteNode(std::shared_ptr<ASTNode> target)
    : target(std::move(target)) {}

Value DeleteNode::eval(EnvPtr env) const {

    auto indexNode = std::dynamic_pointer_cast<IndexNode>(target);

    if (!indexNode) {

        throw SyntaxErrorException("invalid del target");
    }

    Value obj = indexNode->object->eval(env);

    Value idx = indexNode->index->eval(env);

    Value delItem = getAttrValue(obj, "__delitem__");

    call(delItem, {idx}, {}, env);

    return {};
}

QString DeleteNode::toString() const {
    return "del " + target->toString();
}

bool DeleteNode::shouldPrint() const { return false; }