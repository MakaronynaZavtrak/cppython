//
// Created by semyo on 08.07.2026.
//

#ifndef CPPYTHON_EXECUTIONHELPERS_H
#define CPPYTHON_EXECUTIONHELPERS_H

#include <vector>
#include <memory>

#include "ResumeContext.h"
#include "YieldSignal.h"
#include "../ast/ASTNode.h"

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
                carried = block[startIndex]->evalResumable(env, ctx);
                haveCarried = true;
            }
            catch (const YieldSignal&) {
                ctx.recordedPath.insert(ctx.recordedPath.begin(), startIndex);
                throw;
            }

            startIndex++; // нырок завершился нормально — идём дальше как обычно

        } else {
            startIndex++; // это была сама точка yield — пропускаем её
        }
    }

    Value last = haveCarried ? carried : Value();

    for (size_t i = startIndex; i < block.size(); ++i) {
        try {
            last = block[i]->evalResumable(env, ctx);
        }
        catch (const YieldSignal&) {
            ctx.recordedPath.insert(ctx.recordedPath.begin(), i);
            throw;
        }
    }

    return last;
}

#endif //CPPYTHON_EXECUTIONHELPERS_H