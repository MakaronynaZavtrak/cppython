#include "ListValue.h"

#include <qlist.h>
#include <algorithm>

#include "CallRuntime.h"
#include "IteratorValue.h"
#include "Value.h"
#include "../runtime/ProtocolHelpers.h"
//
// Created by semyo on 12.05.2026.
//
QString ListValue::toString() const {

    QStringList parts;

    for (const auto& el : elements) {
        parts << el.repr();
    }

    return "[" + parts.join(", ") + "]";
}

QString ListValue::repr() const {
    return toString();
}

Value ListValue::getItem(const Value& index) {

    if (index.isSlice()) {

        const auto sliceObj = index.asSlice();

        std::vector<Value> result;

        iterateSlice(
            normalizeSlice(
                *sliceObj,
                static_cast<long long>(elements.size())
            ),
            [&](const long long i) {

                result.push_back(
                    elements[static_cast<size_t>(i)]
                );
            }
        );

        return Value(
            std::make_shared<ListValue>(
                std::move(result)
            )
        );
    }

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

void ListValue::append(const Value &value) {
    elements.push_back(value);
}

Value ListValue::pop(const std::optional<Value>& index) {

    if (elements.empty()) {
        throw std::runtime_error("IndexError: pop from empty list");
    }

    std::ptrdiff_t i;

    // pop()
    if (!index.has_value()) {

        i = static_cast<std::ptrdiff_t>(
            elements.size() - 1
        );

    } else {

        i = index->toBigInt().convert_to<long long>();

        // отрицательные индексы
        if (i < 0) {
            i += static_cast<std::ptrdiff_t>(
                elements.size()
            );
        }
    }

    if (i < 0 ||
        i >= static_cast<std::ptrdiff_t>(elements.size())) {

        throw std::runtime_error(
            "IndexError: pop index out of range");
        }

    Value result = elements[i];

    elements.erase(elements.begin() + i);

    return result;
}

std::size_t ListValue::len() const {
    return elements.size();
}

void ListValue::extend(const Value& other) {

    if (!other.isIterable()) {
        throw std::runtime_error("extend expects iterable");
    }

    const auto iter = other.getIterator();

    while (iter->hasNext()) {
        elements.push_back(iter->next());
    }
}

void ListValue::insert(const Value& index,
                       const Value& value) {

    auto i = index.toBigInt().convert_to<long long>();

    // отрицательный индекс
    if (i < 0) {
        i += static_cast<std::ptrdiff_t>(
            elements.size()
        );
    }

    // зажим слева
    if (i < 0) {
        i = 0;
    }

    // зажим справа
    if (i > static_cast<std::ptrdiff_t>(elements.size())) {
        i = static_cast<std::ptrdiff_t>(elements.size());
    }

    elements.insert(elements.begin() + i, value);
}

void ListValue::remove(const Value& value) {

    const auto it = std::find(
        elements.begin(),
        elements.end(),
        value
    );

    if (it == elements.end()) {
        throw std::runtime_error("ValueError: list.remove(x): x not in list");
    }

    elements.erase(it);
}

void ListValue::clear() {
    elements.clear();
}

Value ListValue::count(const Value& value) const {

    std::size_t c = 0;

    for (const auto& e : elements) {
        if (e == value) {
            c++;
        }
    }

    return Value(Value::BigInt(c));
}

Value ListValue::index(
    const Value& value,
    const std::optional<Value>& start,
    const std::optional<Value>& end
) const {

    std::ptrdiff_t s = 0;
    std::ptrdiff_t e = static_cast<std::ptrdiff_t>(elements.size());

    // start
    if (start.has_value()) {

        s = start->toBigInt().convert_to<long long>();

        if (s < 0) {
            s += static_cast<std::ptrdiff_t>(
                elements.size()
            );
        }

        if (s < 0) {
            s = 0;
        }

        if (s > static_cast<std::ptrdiff_t>(elements.size())) {
            s = static_cast<std::ptrdiff_t>(
                elements.size()
            );
        }
    }

    // end
    if (end.has_value()) {

        e = end->toBigInt().convert_to<long long>();

        if (e < 0) {
            e += static_cast<std::ptrdiff_t>(
                elements.size()
            );
        }

        if (e < 0) {
            e = 0;
        }

        if (e > static_cast<std::ptrdiff_t>(elements.size())) {
            e = static_cast<std::ptrdiff_t>(
                elements.size()
            );
        }
    }

    for (std::ptrdiff_t i = s; i < e; ++i) {

        if (elements[i] == value) {
            return Value(
                Value::BigInt(i)
            );
        }
    }

    throw std::runtime_error("ValueError: value is not in list");
}

void ListValue::reverse() {
    std::reverse(elements.begin(),elements.end());
}

Value ListValue::copy() const {
    return Value(std::make_shared<ListValue>(elements));
}

/* TODO: пооддержка kwargs пока отсутсвует
    для вызова reverse = True || False делать просто sort([key, ]True || False)
*/
void ListValue::sort(
    const std::optional<Value>& key,
    bool reverse,
    const std::shared_ptr<Environment>& env) {

    // обычный sort
    if (!key.has_value()) {

        std::sort(
            elements.begin(),
            elements.end(),

            [reverse](const Value& a, const Value& b) {
                return reverse ? b < a : a < b;
            }
        );

        return;
    }

    // decorate
    std::vector<std::pair<Value, Value>> decorated;

    for (const auto& elem : elements) {

        Value k = call(key.value(),{ elem }, {}, env);

        decorated.emplace_back(k, elem);
    }

    // sort by key
    std::sort(
        decorated.begin(),
        decorated.end(),

        [reverse](const auto& a, const auto& b) {
            return reverse
                ? b.first < a.first
                : a.first < b.first;
        }
    );

    // undecorate
    elements.clear();

    for (const auto& [k, v] : decorated) {
        elements.push_back(v);
    }
}

bool ListValue::equal(const Value& other) const {

    if (!other.isList()) {
        return false;
    }

    const auto& rhs = other.asList()->elements;

    if (elements.size() != rhs.size()) {
        return false;
    }

    for (size_t i = 0; i < elements.size(); ++i) {
        if (elements[i] != rhs[i]) {
            return false;
        }
    }

    return true;
}

bool ListValue::notEqual(const Value& other) const {
    return !equal(other);
}

bool ListValue::less(const Value& other) const {

    if (!other.isList()) {
        throw std::runtime_error(
            "TypeError: '<' not supported between instances of 'list' and other type"
        );
    }

    const auto& rhs = other.asList()->elements;

    const size_t minSize =
        std::min(elements.size(), rhs.size());

    for (size_t i = 0; i < minSize; ++i) {

        if (elements[i] == rhs[i]) {
            continue;
        }

        return elements[i] < rhs[i];
    }

    return elements.size() < rhs.size();
}

bool ListValue::lessOrEqual(const Value& other) const {
    return !greater(other);
}

bool ListValue::greater(const Value& other) const {

    if (!other.isList()) {
        throw std::runtime_error(
            "TypeError: '>' not supported between instances of 'list' and other type"
        );
    }

    const auto& rhs = other.asList()->elements;

    const size_t minSize =
        std::min(elements.size(), rhs.size());

    for (size_t i = 0; i < minSize; ++i) {

        if (elements[i] == rhs[i]) {
            continue;
        }

        return elements[i] > rhs[i];
    }

    return elements.size() > rhs.size();
}

bool ListValue::greaterOrEqual(const Value& other) const {
    return !less(other);
}
