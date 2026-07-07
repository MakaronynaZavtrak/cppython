//
// Created by semyo on 07.07.2026.
//

#ifndef CPPYTHON_YIELDSIGNAL_H
#define CPPYTHON_YIELDSIGNAL_H

#include "Value.h"

class YieldSignal {
public:

    Value value{};
    explicit YieldSignal(Value v) : value(std::move(v)) {}
};
#endif //CPPYTHON_YIELDSIGNAL_H