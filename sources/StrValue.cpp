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

Value StrValue::replace(
    const Value& oldValue,
    const Value& newValue,
    const std::optional<Value>& count) const {

    const QString oldStr = oldValue.asString("replace")->toString();
    const QString newStr = newValue.asString("replace")->toString();

    QString result = value;

    // replace all
    if (!count.has_value()) {

        result.replace(oldStr, newStr);

        return Value(result);
    }

    const long long limit = static_cast<long long>(count->asBigInt());

    if (limit < 0) {

        result.replace(oldStr, newStr);

        return Value(result);
    }

    qsizetype pos = 0;
    long long replaced = 0;

    while ((pos = result.indexOf(oldStr, pos)) != -1 && replaced < limit) {

        result.replace(pos, oldStr.size(), newStr);

        pos += newStr.size();

        ++replaced;
    }

    return Value(result);
}

Value StrValue::startswith(
    const Value& prefix,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    QString prefixStr =
        prefix.asString("startwith")->toString();

    qsizetype begin = 0;
    qsizetype finish = value.size();

    if (start.has_value()) {
        begin = static_cast<qsizetype>(start->asBigInt());
    }

    if (end.has_value()) {
        finish = static_cast<qsizetype>(end->asBigInt());
    }

    // clamp
    begin = std::max<qsizetype>(0, begin);
    finish = std::min<qsizetype>(finish, value.size());

    if (begin > finish) {
        begin = finish;
    }

    const QString sliced = value.mid(begin, finish - begin);

    return Value(sliced.startsWith(prefixStr));
}

Value StrValue::endswith(
    const Value& suffix,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    QString suffixStr =
        suffix.asString("endswith")->toString();

    qsizetype begin = 0;
    qsizetype finish = value.size();

    if (start.has_value()) {
        begin = static_cast<qsizetype>(start->toBigInt());
    }

    if (end.has_value()) {
        finish = static_cast<qsizetype>(end->toBigInt());
    }

    // clamp
    begin = std::max<qsizetype>(0, begin);

    finish = std::min<qsizetype>(finish, value.size());

    if (begin > finish) {
        begin = finish;
    }

    const QString sliced = value.mid(begin, finish - begin);

    return Value(sliced.endsWith(suffixStr));
}

Value StrValue::find(
    const Value& sub,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    const QString subStr = sub.asString("find")->toString();

    qsizetype begin = 0;
    qsizetype finish = value.size();

    if (start.has_value()) {
        begin = static_cast<qsizetype>(start->toBigInt());
    }

    if (end.has_value()) {
        finish = static_cast<qsizetype>(end->toBigInt());
    }

    // clamp
    begin = std::max<qsizetype>(0, begin);

    finish = std::min<qsizetype>(finish, value.size());

    if (begin > finish) {
        begin = finish;
    }

    const QString sliced = value.mid(begin, finish - begin);

    const qsizetype idx = sliced.indexOf(subStr);

    if (idx == -1) {
        return Value(Value::BigInt(-1));
    }

    // важно:
    // возвращаем индекс относительно исходной строки
    return Value(Value::BigInt(begin + idx));
}

Value StrValue::count(
    const Value& sub,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    const QString subStr = sub.asString("count")->toString();

    qsizetype begin = 0;
    qsizetype finish = value.size();

    if (start.has_value()) {
        begin = static_cast<qsizetype>(start->toBigInt());
    }

    if (end.has_value()) {
        finish = static_cast<qsizetype>(end->toBigInt() );
    }

    begin = std::max<qsizetype>(0, begin);

    finish = std::min<qsizetype>(finish, value.size());

    if (begin > finish) {
        begin = finish;
    }

    const QString sliced = value.mid(begin, finish - begin);

    // особый случай:
    // Python: "".count("") == 1
    if (subStr.isEmpty()) {
        return Value(Value::BigInt(sliced.size() + 1));
    }

    qsizetype occurrences = 0;
    qsizetype pos = 0;

    while (true) {

        pos = sliced.indexOf(subStr, pos);

        if (pos == -1) {
            break;
        }

        ++occurrences;

        // не пересекающиеся вхождения
        pos += subStr.size();
    }

    return Value(Value::BigInt(occurrences));
}

Value StrValue::index(
    const Value& sub,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    Value result = find(sub, start, end);

    if (result.toBigInt() == -1) {
        throw std::runtime_error("ValueError: substring not found");
    }

    return result;
}
