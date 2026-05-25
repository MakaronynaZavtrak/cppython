//
// Created by semyo on 25.05.2026.
//

#ifndef CPPYTHON_ARGVALIDATION_H
#define CPPYTHON_ARGVALIDATION_H
#include <vector>

#include "Value.h"

inline void expectArgs(
    const std::vector<Value>& args,
    const size_t expected,
    const QString& name) {

    if (args.size() != expected) {

        const QString argGrammar = expected == 1 ? "arg" : "args";

        throw std::runtime_error((name + " expects " +
            QString::number(expected) + " " + argGrammar).toStdString()
        );
    }
}

inline void expectArgsRange(
    const std::vector<Value>& args,
    const size_t min,
    const size_t max,
    const QString& name) {

    if (args.size() < min || args.size() > max) {

        throw std::runtime_error((name + " expects between " + QString::number(min) +
             " and " + QString::number(max) + " args").toStdString());
        }
}
#endif //CPPYTHON_ARGVALIDATION_H