//
// Created by semyo on 25.05.2026.
//

#ifndef CPPYTHON_ARGVALIDATION_H
#define CPPYTHON_ARGVALIDATION_H
#include <vector>

#include "Value.h"
#include "../exception/TypeErrorException.h"

inline void expectArgs(
    const std::vector<Value>& args,
    const size_t expected,
    const QString& name) {

    if (args.size() != expected) {

        const QString argGrammar = expected == 1 ? "arg" : "args";

        throw TypeErrorException(
            name + " expects " +
            QString::number(expected) + " " + argGrammar
        );
    }
}

inline void expectArgsRange(
    const std::vector<Value>& args,
    const size_t min,
    const size_t max,
    const QString& name) {

    if (args.size() < min || args.size() > max) {

        throw TypeErrorException((name + " expects between " + QString::number(min) +
             " and " + QString::number(max) + " args"));
        }
}

inline void expectNoKwargs(const Kwargs& kwargs, const QString& funcName) {

    if (kwargs.empty()) {
        return;
    }

    if (funcName.isEmpty()) {

        throw TypeErrorException(
            "keyword arguments are not supported"
        );
    }

    throw TypeErrorException(
        QString("%1() takes no keyword arguments")
            .arg(funcName)
    );
}
#endif //CPPYTHON_ARGVALIDATION_H