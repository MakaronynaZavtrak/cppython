//
// Created by semyo on 01.06.2026.
//
#include <BytesValue.h>

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

BytesValue::BytesValue(QByteArray data) : data(std::move(data)) {}

const QByteArray& BytesValue::bytes() const {
    return data;
}

QString BytesValue::toString() const {
    return repr();
}
