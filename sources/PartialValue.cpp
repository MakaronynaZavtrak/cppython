//
// Created by semyo on 04.10.2026.
//
#include "PartialValue.h"

QString PartialValue::toString() const {
    return "functools.partial(" + func.toString() + ")";
}