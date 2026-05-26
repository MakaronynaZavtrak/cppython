//
// Created by semyo on 25.05.2026.
//

#include "StrValue.h"

#include <QRegularExpression>

#include "IteratorValue.h"
#include "ListValue.h"
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

Value StrValue::strip(const std::optional<QString>& chars) const {

    QString result = value;

    // default whitespace strip
    if (!chars.has_value()) {
        return Value(result.trimmed());
    }

    const QString& trimChars = *chars;

    qsizetype start = 0;
    qsizetype end = result.size() - 1;

    while (start < result.size() && trimChars.contains(result[start])) {
        ++start;
    }

    while (end >= start && trimChars.contains(result[end])) {
        --end;
    }

    return Value(
        result.mid(start, end - start + 1)
    );
}

Value StrValue::split(const std::optional<QString>& sep,
    std::optional<qsizetype> maxSplit) const {

    QStringList parts;

    // whitespace split
    if (!sep.has_value()) {

        parts = value.split(
            QRegularExpression("\\s+"),
            Qt::SkipEmptyParts
        );

    } else {

        if (sep->isEmpty()) {
            throw std::runtime_error("empty separator");
        }

        if (maxSplit.has_value()) {

            QString remaining = value;
            qsizetype splits = 0;

            while (splits < *maxSplit) {

                const qsizetype idx = remaining.indexOf(*sep);

                if (idx == -1)
                    break;

                parts.append(remaining.left(idx));

                remaining = remaining.mid(idx + sep->size());

                ++splits;
            }

            parts.append(remaining);

        } else {

            parts = value.split(*sep);
        }
    }

    std::vector<Value> result;

    for (const auto& part : parts) {
        result.emplace_back(part);
    }

    return Value(std::make_shared<ListValue>(result));
}

Value StrValue::join(const Value& iterable) const {

    const auto it = iterable.getIterator();

    QStringList parts;

    while (it->hasNext()) {

        Value item = it->next();

        parts.append(item.asString("join iterable")->toString());
    }

    return Value(parts.join(value));
}
