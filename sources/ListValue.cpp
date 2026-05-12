#include "ListValue.h"

#include <qlist.h>

#include "Value.h"
//
// Created by semyo on 12.05.2026.
//
QString ListValue::toString() const {

    QStringList parts;

    for (const auto& el : elements) {
        parts << el.toString();
    }

    return "[" + parts.join(", ") + "]";
}

Value ListValue::getItem(const Value& index) {

    auto i = index.toBigInt();

    if (i < 0) {
        i += static_cast<int>(elements.size());
    }

    if (i < 0 || i >= elements.size()) {
        throw std::runtime_error("IndexError: list index out of range");
    }

    const auto idx = i.convert_to<size_t>();
    return elements[idx];
}

void ListValue::setItem(const Value &index, const Value &value) {
    auto i = index.toBigInt();

    if (i < 0) {
        i += static_cast<long long>(elements.size());
    }

    if (i < 0 || i >= elements.size()) {
        throw std::runtime_error(
            "IndexError: list assignment index out of range"
        );
    }

    elements[i.convert_to<size_t>()] = value;
}
