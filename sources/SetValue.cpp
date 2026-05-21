//
// Created by semyo on 20.05.2026.
//
#include "SetValue.h"
#include "Value.h"

QString SetValue::toString() const {

    if (order.empty()) {
        return "set()";
    }

    QString out = "{";

    bool first = true;

    for (const auto& value : order) {

        if (!first) {
            out += ", ";
        }

        first = false;

        out += value.toString();
    }

    out += "}";

    return out;
}

QString SetValue::repr() const {
    return toString();
}

void SetValue::add(const Value& value) {

    if (!elements.contains(value)) {
        elements[value] = true;
        order.push_back(value);
    }
}

void SetValue::remove(const Value& value) {

    if (!elements.contains(value)) {
        throw std::runtime_error("KeyError: element not found in set");
    }

    elements.remove(value);

    order.removeIf([&](const Value& v) {
        return v == value;
    });
}


