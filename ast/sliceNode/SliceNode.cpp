//
// Created by semyo on 06.07.2026.
//

#include "SliceNode.h"

#include "SliceValue.h"

SliceNode::SliceNode(
        std::shared_ptr<ASTNode> start,
        std::shared_ptr<ASTNode> stop,
        std::shared_ptr<ASTNode> step)
        : start(std::move(start)),
          stop(std::move(stop)),
          step(std::move(step))
{}


Value SliceNode::eval(const EnvPtr env) const {

    std::optional<Value> startValue;
    std::optional<Value> stopValue;
    std::optional<Value> stepValue;

    if (start) {
        startValue = start->eval(env);
    }

    if (stop) {
        stopValue = stop->eval(env);
    }

    if (step) {
        stepValue = step->eval(env);
    }

    return Value(
        std::make_shared<SliceValue>(
            startValue,
            stopValue,
            stepValue
        )
    );

}

QString SliceNode::toString() const {

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