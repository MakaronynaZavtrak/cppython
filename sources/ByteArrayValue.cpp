//
// Created by semyo on 11.06.2026.
//
#include "ByteArrayValue.h"

#include "BytesValue.h"
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

    return QString(
        "bytearray(%1)").arg(
        BytesValue(data).repr()
    );
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
