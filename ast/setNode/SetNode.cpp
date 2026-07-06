//
// Created by semyo on 06.07.2026.
//

#include "SetNode.h"

#include "IteratorValue.h"
#include "SetValue.h"
#include "../starredNode/StarredNode.h"

SetNode::SetNode(std::vector<std::shared_ptr<ASTNode>> elements)
    : elements(std::move(elements)) {}

Value SetNode::eval(EnvPtr env) const {

    const auto set = std::make_shared<SetValue>();

    for (const auto& element : elements) {

        if (const auto starred = dynamic_cast<StarredNode *>(element.get())) {

            Value iterable = starred->value->eval(env);

            const auto iter = iterable.getIterator();

            while (iter->hasNext()) {
                if (Value next = iter->next();
                    !set->elements.contains(next)) {

                    set->elements.insert(next);
                    set->order.push_back(next);
                }
            }
        }
        else {
            if (Value value = element->eval(env);
                !set->elements.contains(value)) {

                set->elements.insert(value);
                set->order.push_back(value);
            }
        }
    }

    return Value(set);
}

QString SetNode::toString() const {
    QStringList parts;

    for (const auto& element : elements) {
        parts << element->toString();
    }
    return "{" + parts.join(", ") + "}";
}