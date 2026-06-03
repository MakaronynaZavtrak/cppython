//
// Created by semyo on 01.06.2026.
//

#ifndef CPPYTHON_BYTESVALUE_H
#define CPPYTHON_BYTESVALUE_H
#include "ObjectValue.h"
#include "Value.h"

class BytesValue : public ObjectValue {

QByteArray data;

public:
    explicit BytesValue(QByteArray data);

    [[nodiscard]] const QByteArray& bytes() const;

    [[nodiscard]] QString toString() const override;
    [[nodiscard]] QString repr() const override;

    [[nodiscard]] Value getItem(const Value& indexValue) const;

    [[nodiscard]] std::size_t len() const;

    [[nodiscard]] Value add(const Value& other) const override;

    [[nodiscard]] Value multiply(const Value &other) const override;

    [[nodiscard]] bool equal(const Value &other) const override;
};
#endif //CPPYTHON_BYTESVALUE_H