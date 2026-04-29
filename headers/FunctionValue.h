//
// Created by semyo on 20.04.2026.
//

#ifndef CPPYTHON_FUNCTIONVALUE_H
#define CPPYTHON_FUNCTIONVALUE_H
#include "Param.h"

class ASTNode;
class Environment;

class FunctionValue {
public:
    std::vector<Param> params;
    std::vector<std::shared_ptr<ASTNode>> body;
    std::shared_ptr<Environment> closure;
};

#endif //CPPYTHON_FUNCTIONVALUE_H