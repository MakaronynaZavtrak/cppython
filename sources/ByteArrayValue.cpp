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
