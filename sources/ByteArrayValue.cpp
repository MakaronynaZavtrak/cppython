//
// Created by semyo on 11.06.2026.
//
#include "ByteArrayValue.h"

#include "BytesValue.h"
#include "ListValue.h"
#include "TupleValue.h"
#include "../runtime/ProtocolHelpers.h"

Value ByteArrayValue::getItem(const Value& indexValue) const {

    if (indexValue.isSlice()) {

        const auto sliceObj = indexValue.asSlice();

        QByteArray result;

        iterateSlice(
            normalizeSlice(*sliceObj, data.size()),
            [&](const long long i) {
                result.append(
                    data[static_cast<int>(i)]
                );
            }
        );

        return Value(
            std::make_shared<ByteArrayValue>(result)
        );
    }

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

std::size_t ByteArrayValue::len() const {
    return data.size();
}

QString ByteArrayValue::repr() const {

    const bool containsSingle = data.contains('\'');

    const QChar quote = containsSingle ? '"' : '\'';

    QString result = "bytearray(b";
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
                result += "\\'";
                break;

            case '"':

                if (quote == '"')
                    result += '"';
                else
                    result += '"';

                break;

            default:

                if (c >= 32 && c <= 126) {

                    result += QChar(c);

                } else {

                    result += QString("\\x%1")
                        .arg(
                            static_cast<int>(c),
                            2,
                            16,
                            QLatin1Char('0')
                        );
                }

                break;
        }
    }

    result += quote;
    result += ")";

    return result;
}

QString ByteArrayValue::toString() const {

    return QString(
        "bytearray(%1)").arg(
        BytesValue(data).toString()
    );
}

Value ByteArrayValue::__bytes__() const {

    return Value(
        std::make_shared<ByteArrayValue>(data)
    );
}

Value ByteArrayValue::add(const Value& other) const {

    if (other.isByteArray()) {

        return Value(
            std::make_shared<ByteArrayValue>(
                data + other.asByteArray()->bytes()
            )
        );
    }

    if (other.isBytes()) {

        return Value(
            std::make_shared<ByteArrayValue>(
                data + other.asBytes()->bytes()
            )
        );
    }

    throw std::runtime_error(
        "TypeError: can't concat bytearray with non-bytes-like object"
    );
}

Value ByteArrayValue::multiply(const Value& other) const {

    if (!other.isBigInt() && !other.isBool()) {

        throw std::runtime_error(
            "TypeError: can't multiply bytearray by non-int"
        );
    }

    const auto count =
        other.toBigInt().convert_to<long long>();

    if (count <= 0) {

        return Value(
            std::make_shared<ByteArrayValue>(
                QByteArray()
            )
        );
    }

    QByteArray result;
    result.reserve(data.size() * count);

    for (long long i = 0; i < count; ++i) {
        result += data;
    }

    return Value(
        std::make_shared<ByteArrayValue>(
            result
        )
    );
}

bool ByteArrayValue::contains(const Value& value) const {

    // int / bool
    if (value.isBigInt() || value.isBool()) {

        const auto v = value.toBigInt();

        if (v < 0 || v > 255) {
            return false;
        }

        return data.contains(
            static_cast<unsigned char>(
                v.convert_to<int>()
            )
        );
    }

    // bytes
    if (value.isBytes()) {

        return data.contains(
            value.asBytes()->bytes()
        );
    }

    // bytearray
    if (value.isByteArray()) {

        return data.contains(
            value.asByteArray()->bytes()
        );
    }

    throw std::runtime_error(
        "TypeError: a bytes-like object or integer is required"
    );
}

bool ByteArrayValue::equal(const Value& other) const {

    if (other.isByteArray()) {

        return data == other.asByteArray("bytearray")->bytes();
    }

    if (other.isBytes()) {

        return data == other.asBytes("bytes")->bytes();
    }

    return false;
}

bool ByteArrayValue::notEqual(const Value& other) const {

    return !equal(other);
}

bool ByteArrayValue::less(const Value& other) const {

    if (other.isByteArray()) {

        return data <
            other.asByteArray("bytearray")->bytes();
    }

    if (other.isBytes()) {

        return data <
            other.asBytes("bytes")->bytes();
    }

    throw std::runtime_error(
        "TypeError: '<' not supported between instances of "
        "'bytearray' and other type"
    );
}

bool ByteArrayValue::lessOrEqual(const Value& other) const {

    if (other.isByteArray()) {

        return data <= other.asByteArray("bytearray")->bytes();
    }

    if (other.isBytes()) {

        return data <= other.asBytes("bytes")->bytes();
    }

    throw std::runtime_error(
        "TypeError: '<=' not supported between instances of "
        "'bytearray' and other type"
    );
}

bool ByteArrayValue::greater(const Value& other) const {

    if (other.isByteArray()) {

        return data >
            other.asByteArray("bytearray")->bytes();
    }

    if (other.isBytes()) {

        return data >
            other.asBytes("bytes")->bytes();
    }

    throw std::runtime_error(
        "TypeError: '>' not supported between instances of "
        "'bytearray' and other type"
    );
}

bool ByteArrayValue::greaterOrEqual(const Value& other) const {

    if (other.isByteArray()) {

        return data >= other.asByteArray("bytearray")->bytes();
    }

    if (other.isBytes()) {

        return data >= other.asBytes("bytes")->bytes();
    }

    throw std::runtime_error(
        "TypeError: '>=' not supported between instances of "
        "'bytearray' and other type"
    );
}

Value ByteArrayValue::find(
    const Value& sub,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    QByteArray needle;

    if (sub.isByteArray()) {

        needle = sub.asByteArray("find")->bytes();

    } else if (sub.isBytes()) {

        needle = sub.asBytes("find")->bytes();

    } else if (sub.isBigInt()) {

        auto v = sub.toBigInt();

        if (v < 0 || v > 255) {

            throw std::runtime_error(
                "byte must be in range(0, 256)"
            );
        }

        needle.append(
            static_cast<char>(
                v.convert_to<int>()
            )
        );

    } else {

        throw std::runtime_error(
            "TypeError: expected bytes-like object"
        );
    }

    long long begin = 0;
    long long finish = data.size();

    if (start.has_value()) {
        begin = start->toBigInt().convert_to<long long>();
    }

    if (end.has_value()) {
        finish = end->toBigInt().convert_to<long long>();
    }

    if (begin < 0) {
        begin += data.size();
    }

    if (finish < 0) {
        finish += data.size();
    }

    begin = std::clamp(begin, 0LL, data.size());
    finish = std::clamp(finish, 0LL, data.size());

    const QByteArray slice =
        data.mid(
            static_cast<int>(begin),
            static_cast<int>(finish - begin)
        );

    const int pos = slice.indexOf(needle);

    if (pos == -1) {
        return Value(Value::BigInt(-1));
    }

    return Value(
        Value::BigInt(begin + pos)
    );
}

Value ByteArrayValue::rfind(
    const Value& sub,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    QByteArray needle;

    if (sub.isByteArray()) {

        needle = sub.asByteArray("rfind")->bytes();

    } else if (sub.isBytes()) {

        needle = sub.asBytes("rfind")->bytes();

    } else if (sub.isBigInt()) {

        auto v = sub.toBigInt();

        if (v < 0 || v > 255) {

            throw std::runtime_error(
                "byte must be in range(0, 256)"
            );
        }

        needle.append(
            static_cast<char>(
                v.convert_to<int>()
            )
        );

    } else {

        throw std::runtime_error(
            "TypeError: expected bytes-like object"
        );
    }

    long long begin = 0;
    long long finish = data.size();

    if (start.has_value()) {
        begin = start->toBigInt().convert_to<long long>();
    }

    if (end.has_value()) {
        finish = end->toBigInt().convert_to<long long>();
    }

    if (begin < 0) {
        begin += data.size();
    }

    if (finish < 0) {
        finish += data.size();
    }

    begin = std::clamp(
        begin,
        0LL,
        static_cast<long long>(data.size())
    );

    finish = std::clamp(
        finish,
        0LL,
        static_cast<long long>(data.size())
    );

    QByteArray slice =
        data.mid(
            static_cast<int>(begin),
            static_cast<int>(finish - begin)
        );

    const int pos = slice.lastIndexOf(needle);

    if (pos == -1) {
        return Value(Value::BigInt(-1));
    }

    return Value(
        Value::BigInt(begin + pos)
    );
}

Value ByteArrayValue::index(
    const Value& sub,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    Value result = find(sub, start, end);

    if (result.toBigInt() == -1) {

        throw std::runtime_error(
            "ValueError: subsection not found"
        );
    }

    return result;
}

Value ByteArrayValue::count(
    const Value& sub,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    QByteArray needle;

    if (sub.isByteArray()) {

        needle = sub.asByteArray("count")->bytes();

    } else if (sub.isBytes()) {

        needle = sub.asBytes("count")->bytes();

    } else if (sub.isBigInt()) {

        auto v = sub.toBigInt();

        if (v < 0 || v > 255) {
            throw std::runtime_error(
                "byte must be in range(0, 256)"
            );
        }

        needle.append(
            static_cast<char>(
                v.convert_to<int>()
            )
        );

    } else {

        throw std::runtime_error(
            "TypeError: expected bytes-like object"
        );
    }

    long long begin = 0;
    long long finish = data.size();

    if (start.has_value()) {
        begin = start->toBigInt().convert_to<long long>();
    }

    if (end.has_value()) {
        finish = end->toBigInt().convert_to<long long>();
    }

    if (begin < 0) {
        begin += data.size();
    }

    if (finish < 0) {
        finish += data.size();
    }

    begin = std::clamp(
        begin,
        0LL,
        data.size()
    );

    finish = std::clamp(
        finish,
        0LL,
        data.size()
    );

    QByteArray haystack =
        data.mid(
            static_cast<int>(begin),
            static_cast<int>(finish - begin)
        );

    if (needle.isEmpty()) {
        return Value(
            Value::BigInt(
                haystack.size() + 1
            )
        );
    }

    int occurrences = 0;
    int pos = 0;

    while (true) {

        pos = haystack.indexOf(needle, pos);

        if (pos == -1) {
            break;
        }

        ++occurrences;

        pos += needle.size();
    }

    return Value(
        Value::BigInt(occurrences)
    );
}

Value ByteArrayValue::startsWith(
    const Value& prefix,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    QByteArray needle;

    if (prefix.isByteArray()) {

        needle = prefix.asByteArray("startswith")->bytes();

    } else if (prefix.isBytes()) {

        needle = prefix.asBytes("startswith")->bytes();

    } else {

        throw std::runtime_error(
            "TypeError: startswith first arg must be bytes-like object"
        );
    }

    long long begin = 0;
    long long finish = data.size();

    if (start.has_value()) {
        begin = start->toBigInt().convert_to<long long>();
    }

    if (end.has_value()) {
        finish = end->toBigInt().convert_to<long long>();
    }

    if (begin < 0) {
        begin += data.size();
    }

    if (finish < 0) {
        finish += data.size();
    }

    begin = std::clamp(
        begin,
        0LL,
        data.size()
    );

    finish = std::clamp(
        finish,
        0LL,
        data.size()
    );

    const QByteArray slice =
        data.mid(
            static_cast<int>(begin),
            static_cast<int>(finish - begin)
        );

    return Value(
        slice.startsWith(needle)
    );
}

Value ByteArrayValue::endsWith(
    const Value& suffix,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    QByteArray needle;

    if (suffix.isByteArray()) {

        needle = suffix.asByteArray("endswith")->bytes();

    } else if (suffix.isBytes()) {

        needle = suffix.asBytes("endswith")->bytes();

    } else {

        throw std::runtime_error(
            "TypeError: endswith first arg must be bytes-like object"
        );
    }

    long long begin = 0;
    long long finish = data.size();

    if (start.has_value()) {
        begin = start->toBigInt().convert_to<long long>();
    }

    if (end.has_value()) {
        finish = end->toBigInt().convert_to<long long>();
    }

    if (begin < 0) {
        begin += data.size();
    }

    if (finish < 0) {
        finish += data.size();
    }

    begin = std::clamp(
        begin,
        0LL,
        static_cast<long long>(data.size())
    );

    finish = std::clamp(
        finish,
        0LL,
        static_cast<long long>(data.size())
    );

    QByteArray slice =
        data.mid(
            static_cast<int>(begin),
            static_cast<int>(finish - begin)
        );

    return Value(
        slice.endsWith(needle)
    );
}

Value ByteArrayValue::lstrip(
const std::optional<Value>& chars) const {

    QByteArray stripChars;

    if (chars.has_value()) {

        const Value& value = *chars;

        if (value.isBytes()) {

            stripChars =
                value.asBytes("lstrip")->bytes();

        } else if (value.isByteArray()) {

            stripChars =
                value.asByteArray("lstrip")->bytes();

        } else {

            throw std::runtime_error(
                "TypeError: lstrip arg must be bytes or bytearray"
            );
        }

    } else {

        stripChars = QByteArray(" \t\n\r\v\f");
    }

    int pos = 0;

    while (
        pos < data.size()
        && stripChars.contains(data[pos])
    ) {
        ++pos;
    }

    return Value(
        std::make_shared<ByteArrayValue>(
            data.mid(pos)
        )
    );
}

Value ByteArrayValue::rstrip(
const std::optional<Value>& chars) const {

    QByteArray stripChars;

    if (chars.has_value()) {

        const Value& value = *chars;

        if (value.isBytes()) {

            stripChars =
                value.asBytes("rstrip")->bytes();

        } else if (value.isByteArray()) {

            stripChars =
                value.asByteArray("rstrip")->bytes();

        } else {

            throw std::runtime_error(
                "TypeError: rstrip arg must be bytes or bytearray"
            );
        }

    } else {

        stripChars = QByteArray(" \t\n\r\v\f");
    }

    int pos = data.size();

    while (
        pos > 0
        && stripChars.contains(data[pos - 1])
    ) {
        --pos;
    }

    return Value(
        std::make_shared<ByteArrayValue>(
            data.left(pos)
        )
    );
}

Value ByteArrayValue::strip(
const std::optional<Value>& chars) const {

    const auto leftStripped = chars.has_value()
        ? lstrip(*chars)
        : lstrip();

    const auto result =
        leftStripped.asByteArray("strip");

    return chars.has_value()
        ? result->rstrip(*chars)
        : result->rstrip();
}

Value ByteArrayValue::removeprefix(const Value& prefix) const {

    QByteArray prefixBytes;

    if (prefix.isBytes()) {

        prefixBytes = prefix.asBytes("removeprefix")->bytes();

    } else if (prefix.isByteArray()) {

        prefixBytes = prefix.asByteArray("removeprefix")->bytes();

    } else {

        throw std::runtime_error(
            "TypeError: removeprefix() argument must be bytes-like"
        );
    }

    if (data.startsWith(prefixBytes)) {

        return Value(
            std::make_shared<ByteArrayValue>(
                data.mid(prefixBytes.size())
            )
        );
    }

    return Value(
        std::make_shared<ByteArrayValue>(data)
    );
}

Value ByteArrayValue::removesuffix(const Value& suffix) const {

    QByteArray suffixBytes;

    if (suffix.isBytes()) {

        suffixBytes = suffix.asBytes("removesuffix")->bytes();

    } else if (suffix.isByteArray()) {

        suffixBytes = suffix.asByteArray("removesuffix")->bytes();

    } else {

        throw std::runtime_error(
            "TypeError: removesuffix() argument must be bytes-like"
        );
    }

    if (
        !suffixBytes.isEmpty()
        && data.endsWith(suffixBytes)
    ) {

        return Value(
            std::make_shared<ByteArrayValue>(
                data.first(
                    data.size() - suffixBytes.size()
                )
            )
        );
    }

    return Value(
        std::make_shared<ByteArrayValue>(data)
    );
}

Value ByteArrayValue::replace(
    const Value& oldValue,
    const Value& newValue,
    const Value::BigInt& count) const {

    QByteArray oldBytes;
    QByteArray newBytes;

    if (oldValue.isBytes()) {

        oldBytes = oldValue.asBytes("replace")->bytes();

    } else if (oldValue.isByteArray()) {

        oldBytes = oldValue.asByteArray("replace")->bytes();

    } else {

        throw std::runtime_error(
            "TypeError: replace() argument 1 must be bytes-like"
        );
    }

    if (newValue.isBytes()) {

        newBytes = newValue.asBytes("replace")->bytes();

    } else if (newValue.isByteArray()) {

        newBytes = newValue.asByteArray("replace")->bytes();

    } else {

        throw std::runtime_error(
            "TypeError: replace() argument 2 must be bytes-like"
        );
    }

    const long long maxCount = count.convert_to<long long>();

    // special case: old == b""
    if (oldBytes.isEmpty()) {

        QByteArray result;

        long long inserted = 0;

        auto canInsert =
            [&]() -> bool {

                return  maxCount < 0 || inserted < maxCount;
            };

        if (canInsert()) {

            result.append(newBytes);
            ++inserted;
        }

        for (const char ch : data) {

            result.append(ch);

            if (canInsert()) {

                result.append(newBytes);
                ++inserted;
            }
        }

        return Value(
            std::make_shared<ByteArrayValue>(
                result
            )
        );
    }

    QByteArray result = data;

    if (maxCount == 0) {

        return Value(
            std::make_shared<ByteArrayValue>(
                result
            )
        );
    }

    int pos = 0;
    long long replaced = 0;

    while ((pos = result.indexOf(oldBytes, pos)) != -1) {

        if (
            maxCount >= 0
            && replaced >= maxCount
        ) {
            break;
        }

        result.replace(
            pos,
            oldBytes.size(),
            newBytes
        );

        pos += newBytes.size();

        ++replaced;
    }

    return Value(
        std::make_shared<ByteArrayValue>(
            result
        )
    );
}

Value ByteArrayValue::split(
    const Value& sep,
    const Value::BigInt& maxsplit) const {

    QByteArray separator;

    if (sep.isBytes()) {

        separator = sep.asBytes("split")->bytes();

    } else if (sep.isByteArray()) {

        separator = sep.asByteArray("split")->bytes();

    } else {

        throw std::runtime_error(
            "TypeError: split() separator must be bytes-like"
        );
    }

    if (separator.isEmpty()) {

        throw std::runtime_error(
            "ValueError: empty separator"
        );
    }

    std::vector<Value> result;

    const long long limit = maxsplit.convert_to<long long>();

    int start = 0;

    long long splits = 0;

    while (true) {

        if (
            limit >= 0
            && splits >= limit
        ) {
            break;
        }

        const int pos = data.indexOf(separator, start);

        if (pos < 0) {
            break;
        }

        result.emplace_back(
            std::make_shared<ByteArrayValue>(
                data.mid(start, pos - start)
            )
        );

        start = pos + separator.size();

        ++splits;
    }

    result.emplace_back(
        std::make_shared<ByteArrayValue>(
            data.mid(start)
        )
    );

    return Value(
        std::make_shared<ListValue>(result)
    );
}

Value ByteArrayValue::rsplit(
    const Value& sep,
    const Value::BigInt& maxsplit) const {

    QByteArray separator;

    if (sep.isBytes()) {

        separator = sep.asBytes("rsplit")->bytes();

    } else if (sep.isByteArray()) {

        separator = sep.asByteArray("rsplit")->bytes();

    } else {

        throw std::runtime_error(
            "TypeError: rsplit() separator must be bytes-like"
        );
    }

    if (separator.isEmpty()) {

        throw std::runtime_error(
            "ValueError: empty separator"
        );
    }

    const long long limit =
        maxsplit.convert_to<long long>();

    std::vector<Value> parts;

    QByteArray remaining = data;

    long long splits = 0;

    while (true) {

        if (
            limit >= 0 &&
            splits >= limit
        ) {
            break;
        }

        const int pos =
            remaining.lastIndexOf(separator);

        if (pos < 0) {
            break;
        }

        parts.emplace_back(
            std::make_shared<ByteArrayValue>(
                remaining.mid(
                    pos + separator.size()
                )
            )
        );

        remaining =
            remaining.left(pos);

        ++splits;
    }

    parts.emplace_back(
        std::make_shared<ByteArrayValue>(
            remaining
        )
    );

    std::reverse(
        parts.begin(),
        parts.end()
    );

    return Value(
        std::make_shared<ListValue>(
            parts
        )
    );
}

Value ByteArrayValue::partition(
    const Value& sep) const {

    QByteArray separator;

    if (sep.isBytes()) {

        separator = sep.asBytes("partition")->bytes();

    } else if (sep.isByteArray()) {

        separator = sep.asByteArray("partition")->bytes();

    } else {

        throw std::runtime_error(
            "TypeError: partition() separator must be bytes-like"
        );
    }

    if (separator.isEmpty()) {

        throw std::runtime_error(
            "ValueError: empty separator"
        );
    }

    const int pos = data.indexOf(separator);

    if (pos < 0) {

        return Value(
            std::make_shared<TupleValue>(
                std::vector{
                    Value(
                        std::make_shared<ByteArrayValue>(data)
                    ),
                    Value(
                        std::make_shared<ByteArrayValue>(
                            QByteArray()
                        )
                    ),
                    Value(
                        std::make_shared<ByteArrayValue>(
                            QByteArray()
                        )
                    )
                }
            )
        );
    }

    return Value(
        std::make_shared<TupleValue>(
            std::vector{

                Value(
                    std::make_shared<ByteArrayValue>(
                        data.left(pos)
                    )
                ),

                Value(
                    std::make_shared<ByteArrayValue>(
                        separator
                    )
                ),

                Value(
                    std::make_shared<ByteArrayValue>(
                        data.mid(
                            pos + separator.size()
                        )
                    )
                )
            }
        )
    );
}

Value ByteArrayValue::rpartition(
    const Value& sep) const {

    QByteArray separator;

    if (sep.isBytes()) {

        separator =
            sep.asBytes("rpartition")->bytes();

    } else if (sep.isByteArray()) {

        separator =
            sep.asByteArray("rpartition")->bytes();

    } else {

        throw std::runtime_error(
            "TypeError: rpartition() separator must be bytes-like"
        );
    }

    if (separator.isEmpty()) {

        throw std::runtime_error(
            "ValueError: empty separator"
        );
    }

    const int pos =
        data.lastIndexOf(separator);

    if (pos < 0) {

        return Value(
            std::make_shared<TupleValue>(
                std::vector<Value>{

                    Value(
                        std::make_shared<ByteArrayValue>(
                            QByteArray()
                        )
                    ),

                    Value(
                        std::make_shared<ByteArrayValue>(
                            QByteArray()
                        )
                    ),

                    Value(
                        std::make_shared<ByteArrayValue>(
                            data
                        )
                    )
                }
            )
        );
    }

    return Value(
        std::make_shared<TupleValue>(
            std::vector<Value>{

                Value(
                    std::make_shared<ByteArrayValue>(
                        data.left(pos)
                    )
                ),

                Value(
                    std::make_shared<ByteArrayValue>(
                        separator
                    )
                ),

                Value(
                    std::make_shared<ByteArrayValue>(
                        data.mid(
                            pos + separator.size()
                        )
                    )
                )
            }
        )
    );
}

Value ByteArrayValue::center(
    const Value::BigInt& width,
    const std::optional<Value>& fillByte) const {

    const auto targetWidth = width.convert_to<long long>();

    if (targetWidth <= data.size()) {

        return Value(
            std::make_shared<ByteArrayValue>(
                data
            )
        );
    }

    char fill = ' ';

    if (fillByte.has_value()) {

        QByteArray fillData;

        if (fillByte->isBytes()) {

            fillData = fillByte->asBytes("center")->bytes();

        } else if (fillByte->isByteArray()) {

            fillData = fillByte->asByteArray("center")->bytes();

        } else {

            throw std::runtime_error(
                "TypeError: center() fill byte must be bytes-like"
            );
        }

        if (fillData.size() != 1) {

            throw std::runtime_error(
                "TypeError: center() fill byte must be length 1"
            );
        }

        fill = fillData[0];
    }

    const auto totalPadding = targetWidth - data.size();

    const auto leftPadding = totalPadding / 2;

    const auto rightPadding = totalPadding - leftPadding;

    QByteArray result;

    result.append(
        QByteArray(leftPadding, fill)
    );

    result.append(data);

    result.append(
        QByteArray(rightPadding, fill)
    );

    return Value(
        std::make_shared<ByteArrayValue>(
            result
        )
    );
}

Value ByteArrayValue::ljust(
    const Value::BigInt& width,
    const std::optional<Value>& fillByte) const {

    const auto targetWidth = width.convert_to<long long>();

    if (targetWidth <= data.size()) {

        return Value(
            std::make_shared<ByteArrayValue>(
                data
            )
        );
    }

    char fill = ' ';

    if (fillByte.has_value()) {

        QByteArray fillData;

        if (fillByte->isBytes()) {

            fillData = fillByte->asBytes("ljust")->bytes();

        } else if (fillByte->isByteArray()) {

            fillData = fillByte->asByteArray("ljust")->bytes();

        } else {

            throw std::runtime_error(
                "TypeError: ljust() fill byte must be bytes-like"
            );
        }

        if (fillData.size() != 1) {

            throw std::runtime_error(
                "TypeError: ljust() fill byte must be length 1"
            );
        }

        fill = fillData[0];
    }

    QByteArray result = data;

    result.append(
        QByteArray(
            targetWidth - data.size(),
            fill
        )
    );

    return Value(
        std::make_shared<ByteArrayValue>(
            result
        )
    );
}

Value ByteArrayValue::rjust(
    const Value::BigInt& width,
    const std::optional<Value>& fillByte) const {

    const auto targetWidth = width.convert_to<long long>();

    if (targetWidth <= data.size()) {

        return Value(
            std::make_shared<ByteArrayValue>(
                data
            )
        );
    }

    char fill = ' ';

    if (fillByte.has_value()) {

        QByteArray fillData;

        if (fillByte->isBytes()) {

            fillData = fillByte->asBytes("rjust")->bytes();

        } else if (fillByte->isByteArray()) {

            fillData =
                fillByte->asByteArray("rjust")->bytes();

        } else {

            throw std::runtime_error(
                "TypeError: rjust() fill byte must be bytes-like"
            );
        }

        if (fillData.size() != 1) {

            throw std::runtime_error(
                "TypeError: rjust() fill byte must be length 1"
            );
        }

        fill = fillData[0];
    }

    QByteArray result;

    result.append(
        QByteArray(
            targetWidth - data.size(),
            fill
        )
    );

    result.append(data);

    return Value(
        std::make_shared<ByteArrayValue>(
            result
        )
    );
}

Value ByteArrayValue::zfill(
    const Value::BigInt& width) const {

    const auto targetWidth = width.convert_to<long long>();

    if (targetWidth <= data.size()) {

        return Value(
            std::make_shared<ByteArrayValue>(
                data
            )
        );
    }

    const auto padding = targetWidth - data.size();

    QByteArray result;

    if (
        !data.isEmpty()
        &&
        (
            data[0] == '+'
            ||
            data[0] == '-'
        )
    ) {

        result.append(data[0]);

        result.append(
            QByteArray(
                padding,
                '0'
            )
        );

        result.append(data.mid(1));

    } else {

        result.append(
            QByteArray(
                padding,
                '0'
            )
        );

        result.append(data);
    }

    return Value(
        std::make_shared<ByteArrayValue>(
            result
        )
    );
}

Value ByteArrayValue::lower() const {

    QByteArray result = data;

    for (char& c : result) {

        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }

    return Value(
        std::make_shared<ByteArrayValue>(result)
    );
}

Value ByteArrayValue::upper() const {

    return Value(
        std::make_shared<ByteArrayValue>(
            data.toUpper()
        )
    );
}

Value ByteArrayValue::swapcase() const {

    QByteArray result = data;

    for (char& c : result) {

        if (c >= 'a' && c <= 'z') {

            c = static_cast<char>(
                c - 'a' + 'A'
            );

        } else if (c >= 'A' && c <= 'Z') {

            c = static_cast<char>(
                c - 'A' + 'a'
            );
        }
    }

    return Value(
        std::make_shared<ByteArrayValue>(
            result
        )
    );
}

Value ByteArrayValue::capitalize() const {

    if (data.isEmpty()) {

        return Value(
            std::make_shared<ByteArrayValue>(
                data
            )
        );
    }

    QByteArray result = data;

    if (
        result[0] >= 'a' &&
        result[0] <= 'z'
    ) {

        result[0] = static_cast<char>(
            result[0] - 'a' + 'A'
        );
    }

    for (int i = 1; i < result.size(); ++i) {

        if (
            result[i] >= 'A' &&
            result[i] <= 'Z'
        ) {

            result[i] = static_cast<char>(
                result[i] - 'A' + 'a'
            );
        }
    }

    return Value(
        std::make_shared<ByteArrayValue>(
            result
        )
    );
}

Value ByteArrayValue::title() const {

    QByteArray result = data;

    bool newWord = true;

    for (char& c : result) {

        const bool isAlpha =
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z');

        if (isAlpha) {

            if (newWord) {

                if (c >= 'a' && c <= 'z') {

                    c = static_cast<char>(
                        c - 'a' + 'A'
                    );
                }

                newWord = false;

            } else {

                if (c >= 'A' && c <= 'Z') {

                    c = static_cast<char>(
                        c - 'A' + 'a'
                    );
                }
            }

        } else {

            newWord = true;
        }
    }

    return Value(
        std::make_shared<ByteArrayValue>(
            result
        )
    );
}

Value ByteArrayValue::isLower() const {

    bool hasLower = false;

    for (const unsigned char c : data) {

        if (c >= 'a' && c <= 'z') {
            hasLower = true;
        }

        if (c >= 'A' && c <= 'Z') {
            return Value(false);
        }
    }

    return Value(hasLower);
}

Value ByteArrayValue::isUpper() const {

    bool hasUpper = false;

    for (const unsigned char c : data) {

        if (c >= 'A' && c <= 'Z') {
            hasUpper = true;
        }

        if (c >= 'a' && c <= 'z') {
            return Value(false);
        }
    }

    return Value(hasUpper);
}
