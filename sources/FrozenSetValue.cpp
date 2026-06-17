//
// Created by semyo on 17.06.2026.
//
#include "FrozenSetValue.h"

std::size_t FrozenSetValue::len() const {
    return elements.size();
}

QString FrozenSetValue::toString() const {
    return repr();
}

QString FrozenSetValue::repr() const {

    QList<Value> sorted = elements.values();

    std::sort(
    sorted.begin(),
    sorted.end(),
    [](const Value& a, const Value& b) {
        try {
            return a < b;
        }
        catch (...) {
            return false;
        }
    });

    QString result = "frozenset(";

    if (!elements.isEmpty()) {

        result += "{";

        bool first = true;

        for (const auto& value : sorted) {

            if (!first)
                result += ", ";

            result += value.repr();

            first = false;
        }

        result += "}";
    }

    result += ")";

    return result;
}