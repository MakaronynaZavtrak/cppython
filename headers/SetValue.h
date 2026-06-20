//
// Created by semyo on 20.05.2026.
//

#ifndef CPPYTHON_SETVALUE_H
#define CPPYTHON_SETVALUE_H
#include <QString>
#include <qset.h>

#include "ObjectValue.h"

class Value;

class SetValue : public ObjectValue, std::enable_shared_from_this<SetValue> {
public:

    explicit SetValue(const QSet<Value>& elements = {}, const QList<Value>& order = {})
        : elements(elements), order(order) {}

    QSet<Value> elements;
    QList<Value> order;

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] QString repr() const override;

    void add(const Value& value);

    void remove(const Value& value);

    void discard(const Value& value);

    [[nodiscard]] bool contains(const Value& value) const override;

    [[nodiscard]] Value unionSet(const std::vector<Value>& others) const;

    [[nodiscard]] Value intersection(const std::vector<Value>& others) const;

    [[nodiscard]] Value difference(const std::vector<Value>& others) const;

    [[nodiscard]] Value symmetricDifference(const Value& other) const;

    [[nodiscard]]bool isSubset(const Value& other) const;

    [[nodiscard]] bool isSuperset(const Value& other) const;

    [[nodiscard]] bool isDisjoint(const Value& other) const;

    [[nodiscard]] Value copy() const;

    void clear();

    Value pop();

    void update(const std::vector<Value>&);

    void differenceUpdate(const std::vector<Value>& others);

    void intersectionUpdate(const std::vector<Value>& others);

    void symmetricDifferenceUpdate(const Value& other);

    [[nodiscard]] std::size_t len() const;

    [[nodiscard]] Value bitOr(const Value& other) const override;

    [[nodiscard]] Value bitAnd(const Value& other) const override;

    [[nodiscard]] Value sub(const Value& other) const override;

    [[nodiscard]] Value bitXor(const Value& other) const override;

    bool equal(const Value &other) const override;

    bool notEqual(const Value& other) const override;

    bool less(const Value& other) const override;
};
#endif //CPPYTHON_SETVALUE_H