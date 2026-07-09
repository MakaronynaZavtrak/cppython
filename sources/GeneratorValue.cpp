#include "GeneratorValue.h"

#include "FunctionValue.h"
#include "../exception/PythonException.h"
#include "../exception/ReturnException.h"
#include "../service/yieldSignal.h"
#include "../ast/ASTNode.h"
#include "../exception/StopIterationException.h"
#include "../service/execBlockResumable.h"

//
// Created by semyo on 07.07.2026.
//

Value GeneratorValue::next() {

    if (finished) {
        throw StopIterationException();
    }

    ResumeContext ctx;
    ctx.replayPath = resumePath;
    ctx.replayCursor = 0;

    ctx.replayIterators = resumeIterators;
    ctx.iterCursor = 0;

    ctx.replayGuardInstances = resumeGuardInstances;
    ctx.guardCursor = 0;

    ctx.replayPendingExceptions = resumePendingExceptions;
    ctx.pendingCursor = 0;

    try {

        Value result = execBlockResumable(func->body, env, ctx);

        finished = true;
        throw StopIterationException();

    }
    catch (const YieldSignal& sig) {
        resumePath = ctx.recordedPath;
        resumeIterators = ctx.recordedIterators;
        resumeGuardInstances = ctx.recordedGuardInstances;
        resumePendingExceptions = ctx.recordedPendingExceptions;
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
