//
// Created by semyo on 17.05.2026.
//
#include "TupleValue.h"

#include "../runtime/ProtocolHelpers.h"

TupleValue::TupleValue(const std::vector<Value>& items)
    : items(items) {}

QString TupleValue::toString() const {

    QString out = "(";

    for (size_t i = 0; i < items.size(); ++i) {

        out += items[i].repr();

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

    if (index.isSlice()) {

        const auto sliceObj = index.asSlice();

        std::vector<Value> result;

        iterateSlice(
            normalizeSlice(*sliceObj, items.size()),
            [&](const long long i) {
                result.push_back(
                    items[static_cast<size_t>(i)]
                );
            }
        );

        return Value(
            std::make_shared<TupleValue>(
                std::move(result)
            )
        );
    }

    if (!index.isBigInt() && !index.isBool()) {
        throw std::runtime_error(
            "TypeError: tuple indices must be integers or slices"
        );
    }

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

Value TupleValue::count(const Value& value) const {

    int count = 0;

    for (const auto& item : items) {

        if (item == value) {
            ++count;
        }
    }

    return Value(Value::BigInt(count));
}

Value TupleValue::index(
    const Value& value,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    std::ptrdiff_t s = 0;
    std::ptrdiff_t e = static_cast<std::ptrdiff_t>(items.size());

    // start
    if (start.has_value()) {

        s = start->toBigInt().convert_to<long long>();

        if (s < 0) {
            s += static_cast<std::ptrdiff_t>(
                items.size()
            );
        }

        if (s < 0) {
            s = 0;
        }

        if (s > static_cast<std::ptrdiff_t>(items.size())) {
            s = static_cast<std::ptrdiff_t>(
                items.size()
            );
        }
    }

    // end
    if (end.has_value()) {

        e = end->toBigInt().convert_to<long long>();

        if (e < 0) {
            e += static_cast<std::ptrdiff_t>(
                items.size()
            );
        }

        if (e < 0) {
            e = 0;
        }

        if (e > static_cast<std::ptrdiff_t>(items.size())) {
            e = static_cast<std::ptrdiff_t>(
                items.size()
            );
        }
    }

    for (std::ptrdiff_t i = s; i < e; ++i) {

        if (items[i] == value) {
            return Value(
                Value::BigInt(i)
            );
        }
    }

    throw std::runtime_error("ValueError: tuple.index(x): x not in tuple");
}

std::size_t TupleValue::len() const {
    return items.size();
}

bool TupleValue::equal(const Value& other) const {

    if (!other.isTuple()) {
        return false;
    }

    const auto& rhs = other.asTuple()->items;

    if (items.size() != rhs.size()) {
        return false;
    }

    for (size_t i = 0; i < items.size(); ++i) {

        if (items[i] != rhs[i]) {
            return false;
        }
    }

    return true;
}

bool TupleValue::notEqual(const Value& other) const {
    return !equal(other);
}

bool TupleValue::lessOrEqual(const Value& other) const {
    return !greater(other);
}

bool TupleValue::less(const Value& other) const {

    if (!other.isTuple()) {
        throw std::runtime_error(
            "TypeError: '<' not supported between instances of 'tuple' and + other type"
        );
    }

    const auto& rhs = other.asTuple()->items;

    const size_t minSize = std::min(items.size(), rhs.size());

    for (size_t i = 0; i < minSize; ++i) {

        if (items[i] == rhs[i]) {
            continue;
        }

        return items[i] < rhs[i];
    }

    return items.size() < rhs.size();
}

bool TupleValue::greaterOrEqual(const Value& other) const {
    return !less(other);
}

bool TupleValue::greater(const Value& other) const {

    if (!other.isTuple()) {
        throw std::runtime_error(
            "TypeError: '>' not supported between instances of 'tuple' and other type"
        );
    }

    const auto& rhs = other.asTuple()->items;

    const size_t minSize =
        std::min(items.size(), rhs.size());

    for (size_t i = 0; i < minSize; ++i) {

        if (items[i] == rhs[i]) {
            continue;
        }

        return items[i] > rhs[i];
    }

    return items.size() > rhs.size();
}
