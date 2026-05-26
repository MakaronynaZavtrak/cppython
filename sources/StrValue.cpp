//
// Created by semyo on 25.05.2026.
//

#include "StrValue.h"

#include "Value.h"

QString StrValue::toString() const {
    return value;
}

QString StrValue::repr() const {
    return value;
}

std::size_t StrValue::len() const {
    return value.size();
}


Value StrValue::getItem(const Value& index) const {

    if (!index.isBigInt()) {
        throw std::runtime_error("TypeError: string indices must be integers");
    }

    auto i = static_cast<long long>(index.toBigInt());

    if (i < 0) {
        i += value.size();
    }

    if (i < 0 || i >= value.size()) {
        throw std::runtime_error("IndexError: string index out of range");
    }

    return Value(
        std::make_shared<StrValue>(
            QString(value[static_cast<int>(i)])
        )
    );
}

Value StrValue::upper() const {
    return Value(value.toUpper());
}

Value StrValue::lower() const {
    return Value(value.toLower());
}
