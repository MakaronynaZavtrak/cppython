//
// Created by semyo on 04.10.2026.
//

#ifndef CPPYTHON_PARTIALVALUE_H
#define CPPYTHON_PARTIALVALUE_H
#include <utility>
#include <vector>
#include "Value.h"

class PartialValue {
public:
    Value func;
    std::vector<Value> args;
    std::vector<std::pair<QString, Value>> keywords;

    PartialValue(Value func,
                 std::vector<Value> args,
                 std::vector<std::pair<QString, Value>> keywords)
        : func(std::move(func)),
          args(std::move(args)),
          keywords(std::move(keywords)) {}

    [[nodiscard]] QString toString() const;
};
#endif //CPPYTHON_PARTIALVALUE_H