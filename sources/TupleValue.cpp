//
// Created by semyo on 17.05.2026.
//
#include "TupleValue.h"
#include "Value.h"

TupleValue::TupleValue(const std::vector<Value>& items)
    : items(items) {}

QString TupleValue::toString() const {

    QString out = "(";

    for (size_t i = 0; i < items.size(); ++i) {

        out += items[i].toString();

        if (i + 1 < items.size()) {
            out += ", ";
        }
    }

    // Python semantics:
    // (1,) вместо (1)

    if (items.size() == 1) {
        out += ",";
    }

    out += ")";

    return out;
}

QString TupleValue::repr() const {
    return toString();
}

Value TupleValue::getItem(const Value& index) const {

    auto i = index.toBigInt();

    if (i < 0) {
        i += static_cast<int>(items.size());
    }

    if (i < 0 || i >= items.size()) {
        throw std::runtime_error("IndexError: tuple index out of range");
    }

    const auto idx = i.convert_to<size_t>();
    return items[idx];
}
