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

    const QByteArray& bytes() const {
        return data;
    }

    QString repr() const override;

    Value getItem(const Value& indexValue) const;

    [[nodiscard]] std::size_t len() const;

    [[nodiscard]] QString toString() const override;

    Value __bytes__() const;

    [[nodiscard]] Value add(const Value& other) const override;
};
#endif //CPPYTHON_BYTEARRAYVALUE_H