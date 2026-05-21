//
// Created by semyo on 20.05.2026.
//

#ifndef CPPYTHON_SETVALUE_H
#define CPPYTHON_SETVALUE_H
#include <QHash>
#include <QString>
#include <QVector>

#include "ReprMixin.h"

class Value;

class SetValue : public ReprMixin {
public:
    QHash<Value, bool> elements;
    QVector<Value> order;

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] QString repr() const override;

    void add(const Value& value);

    void remove(const Value& value);

    void discard(const Value& value);

    [[nodiscard]] std::shared_ptr<SetValue> unionWith(const std::shared_ptr<SetValue>& other) const;

    [[nodiscard]] bool contains(const Value& value) const;

    [[nodiscard]] std::shared_ptr<SetValue> intersectionWith(const std::shared_ptr<SetValue>& other) const;

    [[nodiscard]] std::shared_ptr<SetValue> differenceWith(const std::shared_ptr<SetValue>& other) const;

    [[nodiscard]] std::shared_ptr<SetValue> symmetricDifferenceWith(const std::shared_ptr<SetValue>& other) const;

};
#endif //CPPYTHON_SETVALUE_H