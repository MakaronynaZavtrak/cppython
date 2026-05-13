//
// Created by semyo on 12.05.2026.
//

#ifndef CPPYTHON_LISTVALUE_H
#define CPPYTHON_LISTVALUE_H
#include <vector>

#include "Value.h"
#include "ReprMixin.h"

class ListValue : public ReprMixin {
public:
    std::vector<Value> elements;

    ListValue() = default;

    explicit ListValue(std::vector<Value> elems)
        : elements(std::move(elems)) {}

    [[nodiscard]] QString toString() const override;

    Value getItem(const Value& index);

    void setItem(const Value& index, const Value& value);

    void append(const Value& value);

    Value pop(const std::optional<Value>& index = std::nullopt);

    std::size_t len() const;
};
#endif //CPPYTHON_LISTVALUE_H