//
// Created by semyo on 06.07.2026.
//

#include "ListNode.h"

#include <qlist.h>

#include "IteratorValue.h"
#include "ListValue.h"
#include "../starredNode/StarredNode.h"

ListNode::ListNode(std::vector<std::shared_ptr<ASTNode>> elems)
        : elements(std::move(elems)) {}

Value ListNode::eval(const EnvPtr env) const {

    std::vector<Value> values;

    for (const auto &el: elements) {

        if (const auto starred = dynamic_cast<StarredNode *>(el.get())) {

            Value iterable = starred->value->eval(env);

            const auto iter = iterable.getIterator();

            while (iter->hasNext()) {
                values.push_back(iter->next());
            }
        }
        else {
            values.push_back(el->eval(env));
        }
    }

    return Value(std::make_shared<ListValue>(std::move(values)));
}

QString ListNode::toString() const {

    QStringList parts;

    for (const auto& el : elements) {
        parts << el->toString();
    }

    return "[" + parts.join(", ") + "]";
}