//
// Created by semyo on 06.07.2026.
//

#include "TupleNode.h"

#include <qlist.h>

#include "IteratorValue.h"
#include "TupleValue.h"
#include "../starredNode/StarredNode.h"

TupleNode::TupleNode(std::vector<std::shared_ptr<ASTNode>> elements)
        : elements(std::move(elements)) {}

Value TupleNode::eval(const EnvPtr env) const {

    std::vector<Value> values;

    for (const auto& element : elements) {

        if (const auto starred = dynamic_cast<StarredNode *>(element.get())) {

            Value iterable = starred->value->eval(env);

            const auto iter = iterable.getIterator();

            while (iter->hasNext()) {
                values.push_back(iter->next());
            }
        }
        else {
            values.push_back(element->eval(env));
        }
    }

    return Value(std::make_shared<TupleValue>(std::move(values)));
}

QString TupleNode::toString() const {

    QStringList parts;

    for (const auto& element : elements) {
        parts << element->toString();
    }
    return "(" + parts.join(", ") + ")";
}