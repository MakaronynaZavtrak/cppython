//
// Created by semyo on 11.06.2026.
//

#ifndef CPPYTHON_BYTEARRAYVALUE_H
#define CPPYTHON_BYTEARRAYVALUE_H
#include "BytesValue.h"
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

    const QByteArray& bytes() const {
        return data;
    }

    QString repr() const override;

    Value getItem(const Value& indexValue) const;

    [[nodiscard]] std::size_t len() const;

    [[nodiscard]] QString toString() const override;

    Value __bytes__() const;

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

};
#endif //CPPYTHON_BYTEARRAYVALUE_H