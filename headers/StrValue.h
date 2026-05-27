//
// Created by semyo on 25.05.2026.
//

#ifndef CPPYTHON_STRVALUE_H
#define CPPYTHON_STRVALUE_H
#include "ReprMixin.h"
#include "Value.h"

class Value;

class StrValue : public  ReprMixin {
    QString value;

public:

    explicit StrValue(QString value) : value(std::move(value)) {}

    explicit StrValue(const char* value) : value(value) {}

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] QString repr() const override;

    [[nodiscard]] std::size_t len() const;

    [[nodiscard]] Value getItem(const Value&) const;

    [[nodiscard]] Value upper() const;

    [[nodiscard]] Value lower() const;

    [[nodiscard]] Value strip(const std::optional<QString>& chars = std::nullopt) const;

    [[nodiscard]] Value split(
    const std::optional<QString>& sep = std::nullopt,
    std::optional<qsizetype> maxSplit = std::nullopt) const;

    [[nodiscard]] Value join(const Value& iterable) const;

    [[nodiscard]] Value replace(
    const Value& oldValue,
    const Value& newValue,
    const std::optional<Value>& count = std::nullopt) const;

    [[nodiscard]] Value startswith(
    const Value& prefix,
    const std::optional<Value>& start = std::nullopt,
    const std::optional<Value>& end = std::nullopt) const;

    [[nodiscard]] Value endswith(
    const Value& suffix,
    const std::optional<Value>& start = std::nullopt,
    const std::optional<Value>& end = std::nullopt) const;

    [[nodiscard]] Value find(
    const Value& sub,
    const std::optional<Value>& start = std::nullopt,
    const std::optional<Value>& end = std::nullopt) const;

    [[nodiscard]] Value count(
    const Value& sub,
    const std::optional<Value>& start = std::nullopt,
    const std::optional<Value>& end = std::nullopt) const;

    [[nodiscard]] Value index(
    const Value& sub,
    const std::optional<Value>& start = std::nullopt,
    const std::optional<Value>& end = std::nullopt) const;

    [[nodiscard]] Value rfind(
    const Value& sub,
    const std::optional<Value>& start = std::nullopt,
    const std::optional<Value>& end = std::nullopt) const;

    [[nodiscard]] Value rindex(
    const Value& sub,
    const std::optional<Value>& start = std::nullopt,
    const std::optional<Value>& end = std::nullopt) const;
};
#endif //CPPYTHON_STRVALUE_H