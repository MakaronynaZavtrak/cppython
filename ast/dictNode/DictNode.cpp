//
// Created by semyo on 06.07.2026.
//

#include "DictNode.h"

#include "DictValue.h"

DictNode::DictNode(std::vector<std::shared_ptr<DictElementNode>> items)
        : items(std::move(items)) {}

Value DictNode::eval(const EnvPtr env) const {

    const auto dict = std::make_shared<DictValue>();

    for (const auto& item : items) {
        item->apply(dict, env);
    }

    return Value(dict);
}

[[nodiscard]] QString DictNode::toString() const {

    QStringList parts;

    for (const auto& item : items) {
        parts << item->toString();
    }

    return "{" + parts.join(", ") + "}";
}