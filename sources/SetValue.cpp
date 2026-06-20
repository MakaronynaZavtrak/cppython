//
// Created by semyo on 20.05.2026.
//
#include "SetValue.h"

#include "FrozenSetValue.h"
#include "IteratorValue.h"
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

        out += value.repr();
    }

    out += "}";

    return out;
}

QString SetValue::repr() const {
    return toString();
}

void SetValue::add(const Value& value) {

    if (!elements.contains(value)) {
        elements.insert(value);
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

void SetValue::discard(const Value& value) {
    if (!elements.contains(value)) {
        return;
    }

    elements.remove(value);

    order.removeIf([&](const Value& v) {
        return v == value;
    });
}

Value SetValue::unionSet(const std::vector<Value>& others) const {

    const auto result = std::make_shared<SetValue>();

    for (const auto& value : order) {
        result->add(value);
    }

    for (const auto& iterable : others) {

        const auto it = iterable.getIterator();

        while (it->hasNext()) {
            result->add(it->next());
        }

    }

    return Value(result);
}

bool SetValue::contains(const Value& value) const {
    return elements.contains(value);
}

Value SetValue::intersection(const std::vector<Value>& others) const {

    const auto result = std::make_shared<SetValue>();

    result->elements = elements;
    result->order = order;

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
            }
            else {
                ++iter;
            }
             }
    }

    result->order.removeIf(
        [&](const Value& v) {
            return !result->elements.contains(v);
        }
    );

    return Value(result);
}

Value SetValue::difference(const std::vector<Value>& others) const {

    const auto result = std::make_shared<SetValue>();

    result->elements = elements;
    result->order = order;

    for (const auto& iterable : others) {

        const auto it = iterable.getIterator();

        while (it->hasNext()) {
            result->elements.remove(it->next());
        }
    }

    result->order.removeIf(
        [&](const Value& v) {
            return !result->elements.contains(v);
        }
    );

    return Value(result);
}

Value SetValue::symmetricDifference(const Value& other) const {

    const auto result = std::make_shared<SetValue>();

    result->elements = elements;
    result->order = order;

    result->symmetricDifferenceUpdate(other);

    return Value(result);
}

bool SetValue::isSubset(const Value& other) const {

    const auto it = other.getIterator();

    QSet<Value> otherSet;

    while (it->hasNext()) {
        otherSet.insert(it->next());
    }

    for (const auto& value : elements) {

        if (!otherSet.contains(value)) {
            return false;
        }
    }

    return true;
}

bool SetValue::isSuperset(const Value& other) const {

    const auto it = other.getIterator();

    while (it->hasNext()) {
        if (!elements.contains(it->next())) {
            return false;
        }
    }

    return true;
}

bool SetValue::isDisjoint(const Value& other) const {

    const auto it = other.getIterator();

    QSet<Value> otherSet;

    while (it->hasNext()) {
        otherSet.insert(it->next());
    }

    for (const auto& item : elements) {

        if (otherSet.contains(item)) {
            return false;
        }
    }

    return true;
}

Value SetValue::copy() const {

    return Value(
    std::make_shared<SetValue>(
        elements,
        order
    )
);
}

void SetValue::clear() {

    elements.clear();
    order.clear();
}

Value SetValue::pop() {

    if (order.empty()) {
        throw std::runtime_error("pop from an empty set");
    }

    Value value = order.front();

    order.pop_front();

    elements.remove(value);

    return value;
}

void SetValue::update(const std::vector<Value>& others) {

    for (const auto& iterable : others) {

        const auto it = iterable.getIterator();

        while (it->hasNext()) {
            add(it->next());
        }

    }
}

void SetValue::differenceUpdate(const std::vector<Value>& others) {

    for (const auto& iterable : others) {

        QList<Value> snapshot;

        const auto it = iterable.getIterator();

        while (it->hasNext()) {
            snapshot.push_back(it->next());
        }

        for (const auto& value : snapshot) {
            discard(value);
        }
    }
}

void SetValue::intersectionUpdate(const std::vector<Value>& others) {

    for (const auto& iterable : others) {

        QSet<Value> current;

        const auto it = iterable.getIterator();

        while (it->hasNext()) {
            current.insert(it->next());
        }

        order.removeIf(
            [&](const Value &v) {
                return !current.contains(v);
            });

        QSet<Value> newElements;

        for (const auto& value : order) {
            newElements.insert(value);
        }

        elements = std::move(newElements);

        order.removeIf(
            [&](const Value& v) {
                return !current.contains(v);
            }
        );
    }
}

void SetValue::symmetricDifferenceUpdate(const Value& other) {

    QList<Value> newOrder;
    QSet<Value> newElements;

    QSet<Value> otherSet;
    QList<Value> otherOrder;

    const auto it = other.getIterator();

    while (it->hasNext()) {

        Value value = it->next();

        otherSet.insert(value);
        otherOrder.push_back(value);
    }

    // элементы текущего set, которых нет в other
    for (const auto& value : order) {

        if (!otherSet.contains(value)) {

            newOrder.push_back(value);
            newElements.insert(value);
        }
    }

    // элементы other, которых нет в текущем set
    for (const auto& value : otherOrder) {

        if (!elements.contains(value)) {

            newOrder.push_back(value);
            newElements.insert(value);
        }
    }

    order = std::move(newOrder);
    elements = std::move(newElements);
}

std::size_t SetValue::len() const {
    return order.size();
}

Value SetValue::bitOr(const Value &other) const {
    return unionSet({other});
}

Value SetValue::bitAnd(const Value &other) const {
    return intersection({other});
}

Value SetValue::sub(const Value &other) const {
    return difference({other});
}

Value SetValue::bitXor(const Value &other) const {
    return symmetricDifference(other);
}

bool SetValue::equal(const Value& other) const {

    if (other.isSet()) {

        const auto otherSet = other.asSet();

        if (elements.size() != otherSet->elements.size()) {
            return false;
        }

        for (const auto& value : elements) {

            if (!otherSet->elements.contains(value)) {
                return false;
            }
        }

        return true;
    }

    if (other.isFrozenSet()) {

        const auto frozenPtr = other.asFrozenSet();

        const auto& otherElements = frozenPtr->getElements();

        if (elements.size() != otherElements.size()) {
            return false;
        }

        for (const auto& value : elements) {

            if (!otherElements.contains(value)) {
                return false;
            }
        }

        return true;
    }

    return false;
}

bool SetValue::notEqual(const Value &other) const {
    return !equal(other);
}
