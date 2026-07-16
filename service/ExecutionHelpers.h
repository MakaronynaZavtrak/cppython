//
// Created by semyo on 09.07.2026.
//

#ifndef CPPYTHON_EXECUTIONHELPERS_H
#define CPPYTHON_EXECUTIONHELPERS_H

#include <vector>
#include <memory>

#include "GeneratorControl.h"
#include "InstanceValue.h"
#include "ResumeContext.h"
#include "../service/yieldSignal.h"
#include "../ast/ASTNode.h"
#include "../exception/StopIterationException.h"

inline Value execBlockResumable(
    const std::vector<std::shared_ptr<ASTNode>>& block,
    const std::shared_ptr<Environment>& env,
    ResumeContext& ctx) {

    size_t startIndex = 0;
    Value carried;
    bool haveCarried = false;

    if (ctx.isReplaying()) {

        startIndex = ctx.consumeReplayStep();

        if (ctx.isReplaying()) {

            try {

                if (!Runtime::callStack.empty()) {
                    auto& frame = Runtime::callStack.back();
                    frame.currentLine = block[startIndex]->line;
                    frame.sourceId = block[startIndex]->sourceId;
                    frame.columnCaptured = false;
                }

                carried = block[startIndex]->evalResumable(env, ctx);
                haveCarried = true;
            }
            catch (const YieldSignal&) {
                ctx.recordedPath.insert(ctx.recordedPath.begin(), startIndex);
                throw;
            }

            startIndex++; // нырок завершился нормально — идём дальше как обычно

        } else {

            if (block[startIndex]->isBareYieldStatement()) {

                if (ctx.hasPendingThrow) {
                    throw PythonException(ctx.thrownInstance.asInstance());
                }

            } else {

                if (ctx.hasPendingThrow) {
                    GeneratorControl::pendingThrowInstances.push_back(ctx.thrownInstance);
                } else {
                    GeneratorControl::pendingSendValues.push_back(ctx.consumeSentValue());
                }

                try {

                    if (!Runtime::callStack.empty()) {
                        auto& frame = Runtime::callStack.back();
                        frame.currentLine = block[startIndex]->line;
                        frame.sourceId = block[startIndex]->sourceId;
                        frame.columnCaptured = false;
                    }

                    auto last = block[startIndex]->eval(env);
                }
                catch (...) {
                    GeneratorControl::pendingSendValues.clear();
                    GeneratorControl::pendingThrowInstances.clear();
                    throw;
                }
            }

            startIndex++;
        }
    }

    Value last = haveCarried ? carried : Value();

    for (size_t i = startIndex; i < block.size(); ++i) {
        try {

            if (!Runtime::callStack.empty()) {
                auto& frame = Runtime::callStack.back();
                frame.currentLine = block[i]->line;
                frame.sourceId = block[i]->sourceId;
                frame.columnCaptured = false;
            }

            last = block[i]->evalResumable(env, ctx);
        }
        catch (const YieldSignal&) {
            ctx.recordedPath.insert(ctx.recordedPath.begin(), i);
            throw;
        }
    }

    return last;
}

inline Value extractStopIterationValue(const StopIterationException& e) {
    const auto& fields = e.getInstance()->fields;
    const auto it = fields.find("value");
    return it != fields.end() ? it.value() : Value();
}

#endif //CPPYTHON_EXECUTIONHELPERS_H