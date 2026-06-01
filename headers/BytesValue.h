//
// Created by semyo on 01.06.2026.
//

#ifndef CPPYTHON_BYTESVALUE_H
#define CPPYTHON_BYTESVALUE_H
#include "ReprMixin.h"

class BytesValue : public ReprMixin {

QByteArray data;

public:
    explicit BytesValue(QByteArray data);

    const QByteArray& bytes() const;

    QString toString() const override;
    QString repr() const override;
};
#endif //CPPYTHON_BYTESVALUE_H