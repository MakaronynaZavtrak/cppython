//
// Created by semyo on 08.07.2026.
//

#ifndef CPPYTHON_RESUMECONTEXT_H
#define CPPYTHON_RESUMECONTEXT_H

#include <vector>
#include "Value.h"

class ResumeContext {
public:
    std::vector<size_t> replayPath;
    size_t replayCursor = 0;
    std::vector<size_t> recordedPath;

    std::vector<Value> replayIterators;
    size_t iterCursor = 0;
    std::vector<Value> recordedIterators;

    std::vector<Value> replayGuardInstances;
    size_t guardCursor = 0;
    std::vector<Value> recordedGuardInstances;

    std::vector<std::exception_ptr> replayPendingExceptions;
    size_t pendingCursor = 0;
    std::vector<std::exception_ptr> recordedPendingExceptions;

    Value sentValue;

    [[nodiscard]] bool isReplaying() const {
        return replayCursor < replayPath.size();
    }

    size_t consumeReplayStep() {
        return replayPath[replayCursor++];
    }

    Value consumeReplayIterator() {
        return replayIterators[iterCursor++];
    }

    Value consumeReplayGuardInstance() {
        return replayGuardInstances[guardCursor++];
    }

    std::exception_ptr consumeReplayPendingException() {
        return replayPendingExceptions[pendingCursor++];
    }

    [[nodiscard]] Value consumeSentValue() const { return sentValue; }
};

#endif //CPPYTHON_RESUMECONTEXT_H