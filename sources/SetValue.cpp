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

void SetValue::discard(const Value& value) {
    if (!elements.contains(value)) {
        return;
    }

    elements.remove(value);

    order.removeIf([&](const Value& v) {
        return v == value;
    });
}

std::shared_ptr<SetValue> SetValue::unionWith(const std::shared_ptr<SetValue>& other) const {

    auto result = std::make_shared<SetValue>();

    // сначала текущий set
    for (const auto& value : order) {
        result->add(value);
    }

    // потом второй
    for (const auto& value : other->order) {
        result->add(value);
    }

    return result;
}

bool SetValue::contains(const Value& value) const {
    return elements.contains(value);
}

std::shared_ptr<SetValue> SetValue::intersectionWith(const std::shared_ptr<SetValue>& other) const {

    auto result = std::make_shared<SetValue>();

    for (const auto& value : order) {

        if (other->contains(value)) {
            result->add(value);
        }
    }

    return result;
}

std::shared_ptr<SetValue> SetValue::differenceWith(const std::shared_ptr<SetValue>& other) const {

    auto result = std::make_shared<SetValue>();

    for (const auto& value : order) {

        if (!other->contains(value)) {
            result->add(value);
        }
    }

    return result;
}

std::shared_ptr<SetValue> SetValue::symmetricDifferenceWith(const std::shared_ptr<SetValue>& other) const {

    auto result = std::make_shared<SetValue>();

    // элементы только из текущего set
    for (const auto& value : order) {

        if (!other->contains(value)) {
            result->add(value);
        }
    }

    // элементы только из other
    for (const auto& value : other->order) {

        if (!contains(value)) {
            result->add(value);
        }
    }

    return result;
}

bool SetValue::isSubsetOf(const std::shared_ptr<SetValue>& other) const {

    for (const auto& value : order) {

        if (!other->contains(value)) {
            return false;
        }
    }

    return true;
}

bool SetValue::isSupersetOf(const std::shared_ptr<SetValue>& other) const {

    for (const auto& value : other->order) {

        if (!contains(value)) {
            return false;
        }
    }

    return true;
}

bool SetValue::isDisjointWith(const std::shared_ptr<SetValue>& other) const {

    for (const auto& value : order) {

        if (other->contains(value)) {
            return false;
        }
    }

    return true;
}

std::shared_ptr<SetValue> SetValue::copy() const {

    auto result = std::make_shared<SetValue>();

    for (const auto& value : order) {
        result->add(value);
    }

    return result;
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

void SetValue::update(const std::shared_ptr<SetValue>& other) {

    for (const auto& value : other->order) {
        add(value);
    }
}

void SetValue::differenceUpdate(const std::shared_ptr<SetValue>& other) {

    QVector<Value> snapshot = other->order;

    for (const auto& value : snapshot) {
        discard(value);
    }
}

void SetValue::intersectionUpdate(const std::shared_ptr<SetValue>& other) {

    QVector<Value> newOrder;
    QHash<Value, bool> newElements;

    for (const auto& value : order) {

        if (other->elements.contains(value)) {

            newOrder.push_back(value);
            newElements[value] = true;
        }
    }

    order = std::move(newOrder);
    elements = std::move(newElements);
}

void SetValue::symmetricDifferenceUpdate(const std::shared_ptr<SetValue>& other) {

    QVector<Value> newOrder;
    QHash<Value, bool> newElements;

    // элементы текущего set, которых нет в other
    for (const auto& value : order) {

        if (!other->elements.contains(value)) {

            newOrder.push_back(value);
            newElements[value] = true;
        }
    }

    // элементы other, которых нет в текущем set
    for (const auto& value : other->order) {

        if (!elements.contains(value)) {

            newOrder.push_back(value);
            newElements[value] = true;
        }
    }

    order = std::move(newOrder);
    elements = std::move(newElements);
}
