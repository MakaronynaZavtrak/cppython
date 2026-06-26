//
// Created by semyo on 25.05.2026.
//

#ifndef CPPYTHON_PROTOCOLHELPERS_H
#define CPPYTHON_PROTOCOLHELPERS_H
#include "CallRuntime.h"
#include "ClassUtils.h"
#include "RuntimeUtils.h"
#include "ArgValidation.h"
#include "SliceValue.h"

inline Value makeIterMethodBuiltin(const Value& obj) {
    return makeIterMethod(obj);
}

template<typename ContainerPtr>
Value makeLenMethodBuiltin(const Value &obj) {

    auto container = extract<ContainerPtr>(obj);

    return makeBuiltin(
        "__len__",

        [container](
                    const std::vector<Value> &args,
                    const Kwargs &,
                    const std::shared_ptr<Environment> &)
    -> Value {

            expectArgs(args, 0, "__len__");

            return Value(Value::BigInt(container->len()));
        }
    );
}

struct NormalizedSlice {
    long long start;
    long long stop;
    long long step;
};

inline NormalizedSlice normalizeSlice(
    const SliceValue& slice,
    const long long length) {

    long long step = 1;

    if (slice.step.has_value()) {

        step = slice.step->toBigInt().convert_to<long long>();

        if (step == 0) {
            throw std::runtime_error(
                "ValueError: slice step cannot be zero"
            );
        }
    }

    long long start;
    long long stop;

    if (step > 0) {

        start = slice.start.has_value()
            ? slice.start->toBigInt().convert_to<long long>()
            : 0;

        stop = slice.stop.has_value()
            ? slice.stop->toBigInt().convert_to<long long>()
            : length;

    } else {

        start = slice.start.has_value()
            ? slice.start->toBigInt().convert_to<long long>()
            : length - 1;

        stop = slice.stop.has_value()
            ? slice.stop->toBigInt().convert_to<long long>()
            : -1;
    }

    if (start < 0) {
        start += length;
    }

    if (stop < 0) {

        if (step > 0) {
            stop += length;
        }
        else if (stop != -1) {
            stop += length;
        }
    }

    if (step > 0) {

        start = std::clamp(start, 0LL, length);

        stop = std::clamp(stop, 0LL, length);

    } else {

        if (start >= length) {
            start = length - 1;
        }

        if (start < -1) {
            start = -1;
        }

        if (stop >= length) {
            stop = length - 1;
        }

        if (stop < -1) {
            stop = -1;
        }
    }

    return {start, stop, step};
}

template<typename AppendFn>
void iterateSlice(
    const NormalizedSlice& slice,
    AppendFn append) {

    if (slice.step > 0) {

        for (auto i = slice.start; i < slice.stop; i += slice.step) {
            append(i);
        }

    } else {

        for (auto i = slice.start; i > slice.stop; i += slice.step) {
            append(i);
        }
    }
}

#endif //CPPYTHON_PROTOCOLHELPERS_H