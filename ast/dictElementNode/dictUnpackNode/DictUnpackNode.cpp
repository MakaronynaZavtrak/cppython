//
// Created by semyo on 06.07.2026.
//

#include "DictUnpackNode.h"

#include "DictValue.h"
#include "../../../exception/RuntimeErrorException.h"

DictUnpackNode::DictUnpackNode(std::shared_ptr<ASTNode> value)
        : value(std::move(value)) {}

void DictUnpackNode::apply(const std::shared_ptr<DictValue>& dict, const EnvPtr env) const {

    const auto other = value->eval(env).asDict();

    for (const auto& key : other->getOrder()) {

        dict->setItem(key, other->getElements()[key]);
    }
}

Value DictUnpackNode::eval(EnvPtr env) const {
    throw RuntimeErrorException("DictUnpackNode cannot be evaluated directly");
}

QString DictUnpackNode::toString() const {
    return "**" + value->toString();
}