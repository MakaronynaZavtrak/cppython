//
// Created by semyo on 08.07.2026.
//

#ifndef CPPYTHON_RESUMECONTEXT_H
#define CPPYTHON_RESUMECONTEXT_H

#include <vector>
#include <cstddef>

class ResumeContext {
public:
    std::vector<size_t> replayPath;
    size_t replayCursor = 0;

    std::vector<size_t> recordedPath;

    [[nodiscard]] bool isReplaying() const {
        return replayCursor < replayPath.size();
    }

    size_t consumeReplayStep() {
        return replayPath[replayCursor++];
    }
};

#endif //CPPYTHON_RESUMECONTEXT_H