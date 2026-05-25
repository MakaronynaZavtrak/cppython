//
// Created by semyo on 25.05.2026.
//

#ifndef CPPYTHON_STRVALUE_H
#define CPPYTHON_STRVALUE_H
#include "ReprMixin.h"

class Value;

class StrValue : public  ReprMixin {
    QString value;

public:

    explicit StrValue(QString value) : value(std::move(value)) {}

    explicit StrValue(const char* value) : value(value) {}

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] QString repr() const override;

    [[nodiscard]] std::size_t len() const;

    [[nodiscard]] Value getItem(const Value&) const;

    [[nodiscard]] Value upper() const;
};
#endif //CPPYTHON_STRVALUE_H