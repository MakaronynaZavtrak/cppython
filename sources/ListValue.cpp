#include "ListValue.h"

#include <qlist.h>

#include "Value.h"
//
// Created by semyo on 12.05.2026.
//
QString ListValue::toString() const {

    QStringList parts;

    for (const auto& el : elements) {
        parts << el.toString();
    }

    return "[" + parts.join(", ") + "]";
}