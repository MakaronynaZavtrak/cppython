//
// Created by semyo on 15.05.2026.
//

#ifndef CPPYTHON_DICTVALUE_H
#define CPPYTHON_DICTVALUE_H
#include <QHash>

#include "ReprMixin.h"

class Value;

class DictValue : public ReprMixin {
    QHash<QString, Value> items;
    QVector<QString> order;

public:

    DictValue() = default;

    explicit DictValue(QHash<QString, Value> values);

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] QString repr() const override;

    [[nodiscard]] Value getItem(const Value& key) const;

    void setItem(const Value& key, const Value& value);
};
#endif //CPPYTHON_DICTVALUE_H