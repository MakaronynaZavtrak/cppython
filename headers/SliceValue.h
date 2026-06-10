//
// Created by semyo on 10.06.2026.
//

#ifndef CPPYTHON_SLICEVALUE_H
#define CPPYTHON_SLICEVALUE_H
#include "ObjectValue.h"

class SliceValue : public ObjectValue {
public:

    std::optional<Value> start;
    std::optional<Value> stop;
    std::optional<Value> step;

    SliceValue(
        std::optional<Value> start,
        std::optional<Value> stop,
        std::optional<Value> step
    )
        : start(std::move(start)),
          stop(std::move(stop)),
          step(std::move(step))
    {}

    [[nodiscard]] QString toString() const override {

        QString result = "slice(";

        if (start) {
            result += start->toString();
        }
        else {
            result += "None";
        }

        result += ", ";

        if (stop) {
            result += stop->toString();
        }
        else {
            result += "None";
        }

        result += ", ";

        if (step) {
            result += step->toString();
        }
        else {
            result += "None";
        }

        result += ")";

        return result;

    }
};
#endif //CPPYTHON_SLICEVALUE_H