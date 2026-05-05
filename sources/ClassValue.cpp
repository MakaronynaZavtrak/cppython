#include "ClassValue.h"
//
// Created by semyo on 05.05.2026.
//
QString ClassValue::toString() const {
    return QString("<class '%1.%2'>")
        .arg("__main__", name);
}