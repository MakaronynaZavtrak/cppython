//
// Created by semyo on 15.05.2026.
//

#ifndef CPPYTHON_DICTVALUE_H
#define CPPYTHON_DICTVALUE_H

#include <QHash>

#include "ObjectValue.h"
#include "Value.h"

class DictValue : public ObjectValue, public std::enable_shared_from_this<DictValue> {
    QHash<Value, Value> elements;
    QVector<Value> order;

public:

    DictValue() = default;

    explicit DictValue(const QHash<Value, Value>&, const QVector<Value>&);

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] QString repr() const override;

    [[nodiscard]] Value getItem(const Value& key) const;

    void setItem(const Value& key, const Value& value);

    [[nodiscard]] Value get(const Value&, const Value& = Value()) const;

    [[nodiscard]] std::size_t len() const;

    void clear();

    [[nodiscard]] Value copy() const;

    Value pop(const Value&, const Value* = nullptr);

    void update(const std::shared_ptr<DictValue>&);

    Value setdefault(const Value&, const Value& = Value());

    [[nodiscard]] Value popitem();

    [[nodiscard]] QVector<Value> getOrder() const;

    [[nodiscard]] QHash<Value, Value> getElements() const;

    [[nodiscard]] const QHash<Value, Value>& getElementsRef() const;

    [[nodiscard]] static Value keys(const std::shared_ptr<DictValue>& self);

    [[nodiscard]] static Value values(const std::shared_ptr<DictValue>& self);

    [[nodiscard]] static Value items(const std::shared_ptr<DictValue>& self);

    [[nodiscard]] static Value fromKeys(const Value& iterable, const std::optional<Value>& defaultValue);

    [[nodiscard]] Value bitOr(const Value& other) const override;

    Value ior(const Value& other) override;

    bool equal(const Value& other) const override;

    bool notEqual(const Value& other) const override;

    [[nodiscard]] Value ror(const Value& other) const override;

    [[nodiscard]] bool contains(const Value& key) const override;

};
#endif //CPPYTHON_DICTVALUE_H