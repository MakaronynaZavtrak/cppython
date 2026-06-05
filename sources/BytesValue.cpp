//
// Created by semyo on 01.06.2026.
//
#include <BytesValue.h>

#include "IteratorValue.h"
#include "ListValue.h"

QString BytesValue::repr() const {

    const bool containsSingle = data.contains('\'');
    const bool containsDouble = data.contains('"');

    QChar quote = '\'';

    if (containsSingle && !containsDouble) {
        quote = '"';
    }

    QString result = "b";
    result += quote;

    for (const unsigned char c : data) {

        switch (c) {

            case '\n':
                result += "\\n";
                break;

            case '\r':
                result += "\\r";
                break;

            case '\t':
                result += "\\t";
                break;

            case '\\':
                result += "\\\\";
                break;

            case '\'':

                if (quote == '\'')
                    result += "\\'";
                else
                    result += '\'';

                break;

            case '"':

                if (quote == '"')
                    result += "\\\"";
                else
                    result += '"';

                break;

            default:

                if (c >= 32 && c <= 126) {
                    result += QChar(c);
                } else {
                    result += QString("\\x%1")
                        .arg(c, 2, 16, QLatin1Char('0'));
                }

                break;
        }
    }

    result += quote;

    return result;
}

static void normalizeSliceIndices(int& start, int& end, const int size) {

    if (start < 0) {
        start += size;
    }

    if (end < 0) {
        end += size;
    }

    start = std::clamp(start, 0, size);
    end   = std::clamp(end,   0, size);

    if (end < start) {
        end = start;
    }
}

static std::pair<int, int> getSliceBounds(
    const QByteArray& data,
    const std::optional<Value>& start,
    const std::optional<Value>& end) {
    int startIdx = 0;
    int endIdx = static_cast<int>(data.size());

    if (start.has_value()) {
        startIdx = static_cast<int>(start->toBigInt());
    }

    if (end.has_value()) {
        endIdx = static_cast<int>(end->toBigInt());
    }

    normalizeSliceIndices(
        startIdx,
        endIdx,
        static_cast<int>(data.size())
    );

    return {startIdx, endIdx};
}

static bool isAsciiWhitespace(const unsigned char c) {
    switch (c) {

        case ' ':
        case '\t':
        case '\n':
        case '\r':
        case '\f':
        case '\v':
            return true;

        default:
            return false;
    }
}

static bool isAsciiAlpha(unsigned char ch) {

    return
        (ch >= 'a' && ch <= 'z') ||
        (ch >= 'A' && ch <= 'Z');
}

Value BytesValue::getItem(const Value& indexValue) const {

    int index = indexValue.asBigInt("__getitem__").convert_to<int>();

    if (data.isEmpty()) {
        throw std::runtime_error(
            "IndexError: index out of range"
        );
    }

    if (index < 0) {
        index += data.size();
    }

    if (index < 0 || index >= data.size()) {
        throw std::runtime_error(
            "IndexError: index out of range"
        );
    }

    return Value(
        Value::BigInt(
            static_cast<unsigned char>(data[index])
        )
    );
}

std::size_t BytesValue::len() const {
    return data.size();
}

Value BytesValue::add(const Value& other) const {

    if (!other.isBytes()) {
        throw std::runtime_error(
            "TypeError: can't concat bytes to non-bytes"
        );
    }

    QByteArray result = data;
    result += other.asBytes("__add__")->bytes();

    return Value(
        std::make_shared<BytesValue>(
            std::move(result)
        )
    );
}

Value BytesValue::multiply(const Value &other) const {

    if (!other.isNumeric() || other.isBigFloat()) {
        throw std::runtime_error(
            "TypeError: can't multiply sequence by non-int"
        );
    }

    const Value::BigInt count = other.toBigInt();

    if (count <= 0) {
        return Value(std::make_shared<BytesValue>(QByteArray()));
    }

    if (count > std::numeric_limits<qsizetype>::max()) {
        throw std::runtime_error("String repetition too large");
    }

    QByteArray result;

    for (Value::BigInt i = 0; i < count; ++i) {
        result += data;
    }

    return Value(std::make_shared<BytesValue>(result));
}

bool BytesValue::equal(const Value& other) const {

    if (!other.isBytes()) {
        return false;
    }

    return data == other.asBytes("__eq__")->bytes();
}

bool BytesValue::notEqual(const Value& other) const {

    if (!other.isBytes()) {
        return true;
    }

    return data != other.asBytes("__ne__")->bytes();
}

bool BytesValue::less(const Value& other) const {

    if (!other.isBytes()) {
        throw std::runtime_error(
            "TypeError: '<' not supported between instances of 'bytes' and '" +
            other.toString().toStdString() + "'"
        );
    }

    return data < other.asBytes("__lt__")->bytes();
}

bool BytesValue::lessOrEqual(const Value& other) const {

    if (!other.isBytes()) {
        throw std::runtime_error(
            "TypeError: bytes can only be compared with bytes"
        );
    }

    return data <= other.asBytes("__le__")->bytes();
}

bool BytesValue::greater(const Value& other) const {

    if (!other.isBytes()) {
        throw std::runtime_error(
            "TypeError: bytes can only be compared with bytes"
        );
    }

    return data > other.asBytes("__gt__")->bytes();
}

bool BytesValue::greaterOrEqual(const Value& other) const {

    if (!other.isBytes()) {
        throw std::runtime_error(
            "TypeError: bytes can only be compared with bytes"
        );
    }

    return data >= other.asBytes("__ge__")->bytes();
}

bool BytesValue::contains(const Value& other) const {

    if (other.isNumeric()) {

        const auto byte = other.toBigInt();

        if (byte < 0 || byte > 255) {
            throw std::runtime_error(
                "ValueError: byte must be in range(0, 256)"
            );
        }

        return data.contains(static_cast<unsigned char>(byte));
    }

    if (other.isBytes()) {

        return data.contains(
            other.asBytes()->bytes()
        );
    }

    throw std::runtime_error(
        "TypeError: a bytes-like object is required"
    );
}

Value BytesValue::find(
    const Value& sub,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    if (!sub.isBytes()) {
        throw std::runtime_error("find() argument must be bytes");
    }

    auto [startIdx, endIdx] = getSliceBounds(data, start, end);

    const auto& needle = sub.asBytes()->bytes();

    const QByteArray slice = data.mid(startIdx, endIdx - startIdx);

    const int pos = slice.indexOf(needle);

    if (pos == -1) {
        return Value(Value::BigInt(-1));
    }

    return Value(Value::BigInt(pos + startIdx));
}

Value BytesValue::index(
    const Value& sub,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    Value result = find(sub, start, end);

    if (result.toBigInt() == Value::BigInt(-1)) {
        throw std::runtime_error("ValueError: subsection not found");
    }

    return result;
}

Value BytesValue::count(
    const Value& sub,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    if (!sub.isBytes()) {
        throw std::runtime_error("count() argument must be bytes");
    }

    auto [startIdx, endIdx] = getSliceBounds(data, start, end);

    const QByteArray haystack = data.mid(startIdx, endIdx - startIdx);

    const QByteArray needle = sub.asBytes()->bytes();


    if (needle.isEmpty()) {
        return Value(
            Value::BigInt(haystack.size() + 1)
        );
    }

    int count = 0;
    int pos = 0;

    while (true) {

        pos = haystack.indexOf(needle, pos);

        if (pos == -1) {
            break;
        }

        ++count;

        // неперекрывающиеся вхождения
        pos += needle.size();
    }

    return Value(Value::BigInt(count));
}

Value BytesValue::startsWith(
    const Value& prefix,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    if (!prefix.isBytes()) {
        throw std::runtime_error(
            "startswith first arg must be bytes"
        );
    }

    auto [startIdx, endIdx] = getSliceBounds(data, start, end);

    const QByteArray slice = data.mid(startIdx, endIdx - startIdx);

    const QByteArray& needle = prefix.asBytes()->bytes();

    return Value(slice.startsWith(needle));
}

Value BytesValue::endsWith(
    const Value& suffix,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    const auto [startIdx, endIdx] = getSliceBounds(data, start, end);

    const QByteArray slice = data.mid(startIdx, endIdx - startIdx);

    return Value(
        slice.endsWith(
            suffix.asBytes("endswith")->bytes()
        )
    );
}

Value BytesValue::split(
    const std::optional<Value>& sep,
    const Value::BigInt &maxsplit) const {

    std::vector<Value> result;

    if (!sep.has_value()) {

        qsizetype i = 0;
        Value::BigInt splits = 0;

        while (i < data.size()) {

            while (i < data.size() && isAsciiWhitespace(data[i])) {
                ++i;
            }

            if (i >= data.size()) {
                break;
            }

            if (maxsplit >= 0 && splits >= maxsplit) {

                result.emplace_back(
                    std::make_shared<BytesValue>(
                        data.mid(i)
                    )
                );

                break;
            }

            const qsizetype start = i;

            while (i < data.size() && !isAsciiWhitespace(data[i])) {
                ++i;
            }

            result.emplace_back(
                std::make_shared<BytesValue>(
                    data.mid(start, i - start)
                )
            );

            ++splits;
        }

        return Value(
            std::make_shared<ListValue>(
                std::move(result)
            )
        );
    }

    if (!sep->isBytes()) {
        throw std::runtime_error(
            "split() separator must be bytes"
        );
    }

    const QByteArray delimiter = sep->asBytes()->bytes();

    if (delimiter.isEmpty()) {
        throw std::runtime_error("ValueError: empty separator");
    }

    qsizetype start = 0;
    Value::BigInt splits = 0;

    while (true) {

        if (maxsplit >= 0 && splits >= maxsplit) {
            break;
        }

        const qsizetype pos = data.indexOf(delimiter, start);

        if (pos == -1) {
            break;
        }

        result.emplace_back(
            std::make_shared<BytesValue>(
                data.mid(start, pos - start)
            )
        );

        start = pos + delimiter.size();

        ++splits;
    }

    result.emplace_back(
        std::make_shared<BytesValue>(
            data.mid(start)
        )
    );

    return Value(
        std::make_shared<ListValue>(
            std::move(result)
        )
    );

}

Value BytesValue::join(const Value& iterable) const {

    if (!iterable.isIterable()) {
        throw std::runtime_error(
            "TypeError: can only join an iterable"
        );
    }

    QByteArray result;

    const auto iter = iterable.getIterator();

    bool first = true;

    while (iter->hasNext()) {

        const Value item = iter->next();

        if (!item.isBytes()) {
            throw std::runtime_error(
                "TypeError: sequence item is not bytes"
            );
        }

        if (!first) {
            result += data;
        }

        result += item.asBytes()->bytes();

        first = false;
    }

    return Value(
        std::make_shared<BytesValue>(
            std::move(result)
        )
    );
}

Value BytesValue::replace(
    const Value& oldValue,
    const Value& newValue,
    const Value::BigInt& count) const {

    if (!oldValue.isBytes()) {
        throw std::runtime_error(
            "replace() argument 1 must be bytes"
        );
    }

    if (!newValue.isBytes()) {
        throw std::runtime_error(
            "replace() argument 2 must be bytes"
        );
    }

    const QByteArray oldBytes = oldValue.asBytes()->bytes();

    const QByteArray newBytes = newValue.asBytes()->bytes();

    QByteArray result = data;

    if (count < 0) {

        result.replace(oldBytes, newBytes);

        return Value(
            std::make_shared<BytesValue>(
                std::move(result)
            )
        );
    }

    if (oldBytes.isEmpty()) {

        QByteArray out;

        Value::BigInt replacements = 0;

        for (qsizetype i = 0; i <= data.size(); ++i) {

            if (replacements < count) {
                out += newBytes;
                ++replacements;
            }

            if (i < data.size()) {
                out += data[i];
            }
        }

        return Value(
            std::make_shared<BytesValue>(
                std::move(out)
            )
        );
    }

    Value::BigInt replacements = 0;
    int pos = 0;

    while (replacements < count) {

        pos = result.indexOf(oldBytes, pos);

        if (pos == -1) {
            break;
        }

        result.replace(pos, oldBytes.size(), newBytes);

        pos += newBytes.size();

        ++replacements;
    }

    return Value(
        std::make_shared<BytesValue>(
            std::move(result)
        )
    );
}

Value BytesValue::isAscii() const {

    for (const unsigned char byte : data) {

        if (byte > 127) {
            return Value(false);
        }
    }

    return Value(true);
}

Value BytesValue::isAlpha() const {

    if (data.isEmpty()) {
        return Value(false);
    }

    for (const unsigned char byte : data) {

        if (!isAsciiAlpha(byte)) {
            return Value(false);
        }
    }

    return Value(true);
}

static bool isAsciiDigit(unsigned char ch) {

    return ch >= '0' && ch <= '9';
}

Value BytesValue::isDigit() const {

    if (data.isEmpty()) {
        return Value(false);
    }

    for (unsigned char byte : data) {

        if (!isAsciiDigit(byte)) {
            return Value(false);
        }
    }

    return Value(true);
}

Value BytesValue::isAlnum() const {

    if (data.isEmpty()) {
        return Value(false);
    }

    for (unsigned char byte : data) {

        if (
            !isAsciiAlpha(byte) &&
            !isAsciiDigit(byte)
        ) {
            return Value(false);
        }
    }

    return Value(true);
}

Value BytesValue::isSpace() const {

    if (data.isEmpty()) {
        return Value(false);
    }

    for (const unsigned char byte : data) {

        if (!isAsciiWhitespace(byte)) {
            return Value(false);
        }
    }

    return Value(true);
}

Value BytesValue::isLower() const {

    bool hasCased = false;

    for (const unsigned char c : data) {

        if (c >= 'a' && c <= 'z') {
            hasCased = true;
            continue;
        }

        if (c >= 'A' && c <= 'Z') {
            return Value(false);
        }
    }

    return Value(hasCased);
}

BytesValue::BytesValue(QByteArray data) : data(std::move(data)) {}

const QByteArray& BytesValue::bytes() const {
    return data;
}

QString BytesValue::toString() const {
    return repr();
}
