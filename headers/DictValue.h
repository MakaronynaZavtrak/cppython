//
// Created by semyo on 15.05.2026.
//

#ifndef CPPYTHON_DICTVALUE_H
#define CPPYTHON_DICTVALUE_H

#include "ReprMixin.h"
#include "Value.h"

class DictValue : public ReprMixin {
    QHash<QString, Value> items;
    QVector<QString> order;

public:

    DictValue() = default;

    explicit DictValue(const QHash<QString, Value>&, const QVector<QString>&);

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] QString repr() const override;

    [[nodiscard]] Value getItem(const Value& key) const;

    void setItem(const Value& key, const Value& value);

    [[nodiscard]] Value get(const Value&, const Value& = Value()) const;

    [[nodiscard]] std::size_t len() const;

    void clear();

    [[nodiscard]] Value copy() const;

    Value pop(const QString&, const Value* = nullptr);

    void update(const std::shared_ptr<DictValue>&);

    Value setdefault(const QString&, const Value& = Value());

    [[nodiscard]] Value popitem();

    [[nodiscard]] QVector<QString> getOrder() const;

    [[nodiscard]] QHash<QString, Value> getItems() const;

    [[nodiscard]] static Value keys(const std::shared_ptr<DictValue>& self);

    [[nodiscard]] static Value values(const std::shared_ptr<DictValue>& self);

};
#endif //CPPYTHON_DICTVALUE_H