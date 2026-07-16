//
// Created by semyo on 06.07.2026.
//

#include "DictPairNode.h"

#include "DictValue.h"
#include "../../../exception/RuntimeErrorException.h"

DictPairNode::DictPairNode(std::shared_ptr<ASTNode> key,
                           std::shared_ptr<ASTNode> value)
        : key(std::move(key)),
          value(std::move(value)) {}

void DictPairNode::apply(const std::shared_ptr<DictValue>& dict, const EnvPtr env) const {
    dict->setItem(key->eval(env), value->eval(env));
}

Value DictPairNode::eval(EnvPtr) const {
    throw RuntimeErrorException("DictKeyValueNode cannot be evaluated directly");
}

QString DictPairNode::toString() const {
    return key->toString() + ": " + value->toString();
}