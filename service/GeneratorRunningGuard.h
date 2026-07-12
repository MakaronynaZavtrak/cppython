//
// Created by semyo on 12.07.2026.
//

#ifndef CPPYTHON_GENERATORRUNNINGGUARD_H
#define CPPYTHON_GENERATORRUNNINGGUARD_H
#include "GeneratorValue.h"

class GeneratorRunningGuard {
    GeneratorValue* gen;
public:
    explicit GeneratorRunningGuard(GeneratorValue* g) : gen(g) {
        gen->running = true;
    }
    ~GeneratorRunningGuard() {
        gen->running = false;
    }
    GeneratorRunningGuard(const GeneratorRunningGuard&) = delete;
    GeneratorRunningGuard& operator=(const GeneratorRunningGuard&) = delete;
};
#endif //CPPYTHON_GENERATORRUNNINGGUARD_H