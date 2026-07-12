#include "GeneratorValue.h"

#include "FunctionValue.h"
#include "../exception/PythonException.h"
#include "../exception/ReturnException.h"
#include "../service/yieldSignal.h"
#include "../ast/ASTNode.h"
#include "../exception/StopIterationException.h"
#include "../exception/TypeErrorException.h"
#include "../service/ExecutionHelpers.h"

//
// Created by semyo on 07.07.2026.
//

Value GeneratorValue::next() {
    return send(Value());
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

Value GeneratorValue::send(const Value& value) {

    if (!started && !value.isNone()) {
        throw TypeErrorException(
            "can't send non-None value to a just-started generator"
        );
    }

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
    ctx.sentValue = value;

    started = true;

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
    catch (const ReturnException& e) {
        finished = true;
        throw StopIterationException(e.getValue());
    }
}

Value GeneratorValue::throwInto(const Value& excValue) {

    Value::InstancePtr instance;

    if (excValue.isClass()) {
        instance = PythonException::makeInstance(excValue.asClass(), "");
    } else if (excValue.isInstance()) {
        instance = excValue.asInstance();
    } else {
        throw TypeErrorException("exceptions must derive from BaseException");
    }

    if (!PythonException::isSubclass(instance->klass, Runtime::baseExceptionClass)) {
        throw TypeErrorException("exceptions must derive from BaseException");
    }

    if (!started || finished) {
        finished = true;
        throw PythonException(instance);
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
    ctx.hasPendingThrow = true;
    ctx.thrownInstance = Value(instance);

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
    catch (const ReturnException& e) {
        finished = true;
        throw StopIterationException(e.getValue());
    }
    catch (const PythonException&) {
        finished = true; // не поймано внутри генератора — закрываем его
        throw;
    }
}
