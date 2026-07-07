#include "GeneratorValue.h"

#include "FunctionValue.h"
#include "../exception/PythonException.h"
#include "../exception/ReturnException.h"
#include "../service/yieldSignal.h"
#include "../ast/ASTNode.h"
#include "../exception/StopIterationException.h"

//
// Created by semyo on 07.07.2026.
//

Value GeneratorValue::next() {

    if (finished) {
        throw StopIterationException();
    }

    size_t i = resumeIndex;

    try {

        for (; i < func->body.size(); ++i) {
            [[maybe_unused]] auto _ = func->body[i]->eval(env);
        }

        finished = true;
        throw StopIterationException();

    }
    catch (const YieldSignal& sig) {
        resumeIndex = i + 1;
        return sig.value;
    }
    catch (const ReturnException&) {
        finished = true;
        throw StopIterationException();
    }
}

bool GeneratorValue::hasNext() const {
    return !finished;
}

QString GeneratorValue::getTypeName() const {
    return "generator";
}

QString GeneratorValue::toString() const {
    return "<generator object " + func->name + " at " +
        QString::number(reinterpret_cast<std::uintptr_t>(this), 16) + ">";
}

QString GeneratorValue::repr() const {
    return toString();
}
