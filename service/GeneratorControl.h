//
// Created by semyo on 09.07.2026.
//

#ifndef CPPYTHON_GENERATORCONTROL_H
#define CPPYTHON_GENERATORCONTROL_H
#include <vector>
#include "Value.h"

class GeneratorControl {
public:
    static std::vector<Value> pendingSendValues;
    static std::vector<Value> pendingThrowInstances;
};
#endif