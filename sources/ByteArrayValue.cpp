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
