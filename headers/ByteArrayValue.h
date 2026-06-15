//
// Created by semyo on 11.06.2026.
//

#ifndef CPPYTHON_BYTEARRAYVALUE_H
#define CPPYTHON_BYTEARRAYVALUE_H
#include "ObjectValue.h"
#include "Value.h"

class ByteArrayValue : public ObjectValue {

    QByteArray data;

public:

    explicit ByteArrayValue(QByteArray data)
        : data(std::move(data))
    {}

    QByteArray& bytes() {
        return data;
    }

    [[nodiscard]] const QByteArray& bytes() const {
        return data;
    }

    [[nodiscard]] QString repr() const override;

    [[nodiscard]] Value getItem(const Value& indexValue) const;

    [[nodiscard]] std::size_t len() const;

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] Value __bytes__() const;

    [[nodiscard]] Value add(const Value& other) const override;

    [[nodiscard]] Value multiply(const Value& other) const override;

    [[nodiscard]] bool contains(const Value& value) const override;

    [[nodiscard]] bool equal(const Value& other) const override;

    [[nodiscard]] bool notEqual(const Value& other) const override;

    [[nodiscard]] bool less(const Value& other) const override;

    [[nodiscard]] bool lessOrEqual(const Value& other) const override;

    [[nodiscard]] bool greater(const Value& other) const override;

    [[nodiscard]] bool greaterOrEqual(const Value& other) const override;

    [[nodiscard]] Value find(
    const Value& sub,
    const std::optional<Value>& start = std::nullopt,
    const std::optional<Value>& end = std::nullopt) const;

    [[nodiscard]] Value rfind(
    const Value& sub,
    const std::optional<Value>& start = std::nullopt,
    const std::optional<Value>& end = std::nullopt) const;

    [[nodiscard]] Value index(
    const Value& sub,
    const std::optional<Value>& start = std::nullopt,
    const std::optional<Value>& end = std::nullopt) const;

    [[nodiscard]] Value count(
    const Value& sub,
    const std::optional<Value>& start = std::nullopt,
    const std::optional<Value>& end = std::nullopt) const;

    [[nodiscard]] Value startsWith(
    const Value& prefix,
    const std::optional<Value>& start = std::nullopt,
    const std::optional<Value>& end = std::nullopt) const;

    [[nodiscard]] Value endsWith(
    const Value& suffix,
    const std::optional<Value>& start = std::nullopt,
    const std::optional<Value>& end = std::nullopt) const;

    [[nodiscard]] Value lstrip(
    const std::optional<Value>& chars = std::nullopt) const;

    [[nodiscard]] Value rstrip(
    const std::optional<Value>& chars = std::nullopt) const;

    [[nodiscard]] Value strip(
    const std::optional<Value>& chars = std::nullopt) const;

    [[nodiscard]] Value removeprefix(const Value& prefix) const;

    [[nodiscard]] Value removesuffix(const Value& suffix) const;

    [[nodiscard]] Value replace(
    const Value& oldValue,
    const Value& newValue,
    const Value::BigInt& count = -1) const;

    [[nodiscard]] Value split(
    const Value& sep,
    const Value::BigInt& maxsplit = -1) const;

    [[nodiscard]] Value rsplit(
    const Value& sep,
    const Value::BigInt& maxsplit = -1) const;

    [[nodiscard]] Value partition(
    const Value& sep) const;

    [[nodiscard]] Value rpartition(
    const Value& sep) const;

    [[nodiscard]] Value center(
    const Value::BigInt& width,
    const std::optional<Value>& fillByte = std::nullopt) const;

    [[nodiscard]] Value ljust(
    const Value::BigInt& width,
    const std::optional<Value>& fillByte = std::nullopt) const;

    [[nodiscard]] Value rjust(
    const Value::BigInt& width,
    const std::optional<Value>& fillByte = std::nullopt) const;

    [[nodiscard]] Value zfill(
    const Value::BigInt& width) const;

    [[nodiscard]] Value lower() const;

    [[nodiscard]] Value upper() const;

    [[nodiscard]] Value swapcase() const;

    [[nodiscard]] Value capitalize() const;

    [[nodiscard]] Value title() const;

    [[nodiscard]] Value isLower() const;

    [[nodiscard]] Value isUpper() const;

    [[nodiscard]] Value isTitle() const;

    [[nodiscard]] Value isAscii() const;

    [[nodiscard]] Value isAlpha() const;

    [[nodiscard]] Value isDigit() const;

    [[nodiscard]] Value isAlnum() const;

    [[nodiscard]] Value isSpace() const;

    [[nodiscard]] Value expandTabs(
    const Value::BigInt& tabsize = 8) const;

    [[nodiscard]] Value splitLines(
    bool keepEnds = false) const;

    [[nodiscard]] Value join(const Value& iterable) const;

    Value append(const Value& value);

    Value extend(const Value& iterable);

    Value insert(const Value::BigInt& indexValue, const Value& value);

    Value pop(const std::optional<Value::BigInt>& index = std::nullopt);

    Value remove(const Value& value);

    Value clear();

    [[nodiscard]] Value copy() const;

    Value reverse();

    [[nodiscard]] Value hex(
    const std::optional<QString>& sep = std::nullopt,
    const std::optional<Value::BigInt>& bytesPerSep = std::nullopt) const;

    static Value maketrans(const std::vector<Value>& args);

    [[nodiscard]] Value decode(
    const QString& encoding = "utf-8",
    const QString& errors = "strict") const;

};
#endif //CPPYTHON_BYTEARRAYVALUE_H