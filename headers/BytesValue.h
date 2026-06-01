//
// Created by semyo on 01.06.2026.
//

#ifndef CPPYTHON_BYTESVALUE_H
#define CPPYTHON_BYTESVALUE_H
#include "ReprMixin.h"
#include "Value.h"

class BytesValue : public ReprMixin {

QByteArray data;

public:
    explicit BytesValue(QByteArray data);

    const QByteArray& bytes() const;

    QString toString() const override;
    QString repr() const override;

    [[nodiscard]] Value getItem(const Value& indexValue) const;

    [[nodiscard]] std::size_t len() const;
};
#endif //CPPYTHON_BYTESVALUE_H