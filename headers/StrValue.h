//
// Created by semyo on 25.05.2026.
//

#ifndef CPPYTHON_STRVALUE_H
#define CPPYTHON_STRVALUE_H
#include "CallRuntime.h"
#include "ObjectValue.h"
#include "Value.h"

class Value;

class StrValue : public ObjectValue, public std::enable_shared_from_this<StrValue> {
    QString value;

public:

    explicit StrValue(QString value) : value(std::move(value)) {}

    explicit StrValue(const char* value) : value(value) {}

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] QString repr() const override;

    [[nodiscard]] std::size_t len() const;

    [[nodiscard]] Value getItem(const Value&) const override;

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

    [[nodiscard]] Value capitalize() const;

    [[nodiscard]] Value title() const;

    [[nodiscard]] Value swapcase() const;

    [[nodiscard]] Value isalpha() const;

    [[nodiscard]] Value isdigit() const;

    [[nodiscard]] Value isalnum() const;

    [[nodiscard]] Value isspace() const;

    static QString escapeString(const QString& str);

    [[nodiscard]] Value center(
    const Value& width,
    const std::optional<Value>& fillCharValue = std::nullopt) const;

    [[nodiscard]] Value ljust(
    const Value& widthValue,
    const std::optional<Value>& fillCharValue = std::nullopt) const;

    [[nodiscard]] Value rjust(
    const Value& widthValue,
    const std::optional<Value>& fillCharValue = std::nullopt) const;

    [[nodiscard]] Value lstrip(
    const std::optional<QString>& chars = std::nullopt) const;

    [[nodiscard]] Value rstrip(
    const std::optional<QString>& chars = std::nullopt) const;

    [[nodiscard]] Value islower() const;

    [[nodiscard]] Value isupper() const;


    //TODO: метод неполноценный, так как не поддерживает unicode
    [[nodiscard]] Value isdecimal() const;

    //TODO: метод неполноценный, так как не поддерживает unicode
    [[nodiscard]] Value isnumeric() const;

    [[nodiscard]] Value istitle() const;

    [[nodiscard]] Value isASCII() const;

    [[nodiscard]] Value isidentifier() const;

    [[nodiscard]] Value isprintable() const;

    [[nodiscard]] Value partition(const Value& sep) const;

    [[nodiscard]] Value rpartition(const Value& sep) const;

    [[nodiscard]] Value splitlines(
    const std::optional<Value>& keepEnds = std::nullopt) const;

    [[nodiscard]] Value zfill(const Value& widthValue) const;

    [[nodiscard]] Value expandtabs(
    const std::optional<Value>& tabsizeValue = std::nullopt) const;

    [[nodiscard]] Value rsplit(
    const std::optional<QString>& sep = std::nullopt,
    std::optional<qsizetype> maxSplit = std::nullopt) const;

    [[nodiscard]] Value casefold() const;

    static Value maketrans(const std::vector<Value>& args);

    [[nodiscard]] Value translate(const Value& table) const;

    [[nodiscard]] Value formatMap(const Value& mapping) const;

    [[nodiscard]] Value format(
    const std::vector<Value>& args,
    const Kwargs& kwargs) const;

    [[nodiscard]] Value add(const Value&) const override;

    [[nodiscard]] Value multiply(const Value&) const override;

    [[nodiscard]] Value rmul(const Value&) const override;

    [[nodiscard]] bool equal(const Value&) const override;

    [[nodiscard]] bool notEqual(const Value&) const override;

    [[nodiscard]] bool lessOrEqual(const Value&) const override;

    [[nodiscard]] bool less(const Value&) const override;

    [[nodiscard]] bool greaterOrEqual(const Value&) const override;

    [[nodiscard]] bool greater(const Value&) const override;

    [[nodiscard]] bool contains(const Value& val) const override;

    [[nodiscard]] std::size_t hash() const override;

    [[nodiscard]] Value mod(const Value& rhs) const override;

    [[nodiscard]] Value formatSelf(const QString& spec) const;

    [[nodiscard]] Value encode(
    const std::optional<QString>& encoding = std::nullopt,
    const std::optional<QString>& errors = std::nullopt) const;

    [[nodiscard]] Value removePrefix(const Value& prefix) const;

    [[nodiscard]] Value removeSuffix(const Value& suffix) const;

private:

    static Value resolveFormatField(
    const std::shared_ptr<DictValue>& dict,
    const QString& field);

    static Value getItemValue(const Value& obj, const Value& key);

    static Value resolveEmptyFormatField(const std::shared_ptr<DictValue>& dict);

    static QString applyFormatSpec(
    const Value& value,
    const QString& spec);

    static QString asciiRepr(const Value &value);

    static QString applyPercentSpec(
    const Value& value,
    const QString& spec,
    int precision);

    static QString formatPercentValue(
    const Value& val,
    const QString& spec,
    bool leftAlign,
    bool zeroPad,
    bool showSign,
    bool spaceSign,
    bool alternateForm,
    int width,
    int precision);

    static QString applyStringFormatSpec(const QString &text, const QString &spec);

    [[nodiscard]] Value modTuple(const std::shared_ptr<TupleValue>& tuple) const;

    [[nodiscard]] Value modSingle(const Value &rhs) const;

    [[nodiscard]] Value modMapping(const Value::DictPtr & dict) const;
};
#endif //CPPYTHON_STRVALUE_H