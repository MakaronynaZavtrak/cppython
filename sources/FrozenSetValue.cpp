//
// Created by semyo on 17.06.2026.
//
#include "FrozenSetValue.h"

#include <QDebug>

#include "IteratorValue.h"

std::size_t FrozenSetValue::len() const {
    return elements.size();
}

bool FrozenSetValue::contains(const Value& value) const {
    return elements.contains(value);
}

const QSet<Value> & FrozenSetValue::getElements() const {
    return elements;
}

const QList<Value> & FrozenSetValue::getOrder() const {
    return order;
}

Value FrozenSetValue::unionSet(const std::vector<Value>& others) const {

    const auto result = std::make_shared<FrozenSetValue>();

    result->elements = elements;

    for (const auto& iterable : others) {

        const auto it = iterable.getIterator();

        while (it->hasNext()) {

            result->elements.insert(
                it->next()
            );
        }
    }

    return Value(result);
}

Value FrozenSetValue::intersection(const std::vector<Value>& others) const {

    const auto result = std::make_shared<FrozenSetValue>();

    result->elements = elements;

    for (const auto& iterable : others) {

        QSet<Value> current;

        const auto it = iterable.getIterator();

        while (it->hasNext()) {
            current.insert(it->next());
        }

        for (auto iter = result->elements.begin();
             iter != result->elements.end();) {

            if (!current.contains(*iter)) {

                iter = result->elements.erase(iter);

            } else {
                ++iter;
            }
        }
    }

    return Value(result);
}

Value FrozenSetValue::difference(const std::vector<Value>& others) const {

    const auto result = std::make_shared<FrozenSetValue>();

    result->elements = elements;

    for (const auto& iterable : others) {

        const auto it = iterable.getIterator();

        while (it->hasNext()) {

            result->elements.remove(it->next());
        }
    }

    return Value(result);
}

Value FrozenSetValue::symmetricDifference(const Value& other) const {

    const auto result = std::make_shared<FrozenSetValue>();

    result->elements = elements;

    const auto it = other.getIterator();

    while (it->hasNext()) {

        const Value value = it->next();

        if (result->elements.contains(value)) {

            result->elements.remove(value);

        } else {

            result->elements.insert(value);
        }
    }

    return Value(result);
}

Value FrozenSetValue::bitOr(const Value& other) const {
    return unionSet({other});
}

Value FrozenSetValue::bitAnd(const Value& other) const {
    return intersection({other});
}

Value FrozenSetValue::sub(const Value& other) const {
    return difference({other});
}

Value FrozenSetValue::bitXor(const Value& other) const {
    return symmetricDifference(other);
}

Value FrozenSetValue::isSubset(const Value& other) const {

    if (!other.isFrozenSet()) {
        throw std::runtime_error(
            "TypeError: expected frozenset"
        );
    }

    const auto otherSet = other.asFrozenSet()->getElements();

    for (const auto& value : elements) {

        if (!otherSet.contains(value)) {
            return Value(false);
        }
    }

    return Value(true);
}

Value FrozenSetValue::isSuperset(const Value& other) const {

    if (!other.isFrozenSet()) {
        throw std::runtime_error(
            "TypeError: expected frozenset"
        );
    }

    const auto otherSet = other.asFrozenSet()->getElements();

    for (const auto& value : otherSet) {

        if (!elements.contains(value)) {
            return Value(false);
        }
    }

    return Value(true);
}

Value FrozenSetValue::isDisjoint(const Value &other) const {

    if (!other.isFrozenSet()) {
        throw std::runtime_error(
            "TypeError: expected frozenset"
        );
    }

    const auto otherSet = other.asFrozenSet();

    for (const auto& item : elements) {

        if (otherSet->contains(item)) {
            return Value(false);
        }
    }

    return Value(true);

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