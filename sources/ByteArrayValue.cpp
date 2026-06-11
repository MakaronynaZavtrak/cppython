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
