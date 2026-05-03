//
// Created by semyo on 20.04.2026.
//

#ifndef CPPYTHON_FUNCTIONVALUE_H
#define CPPYTHON_FUNCTIONVALUE_H
#include "InstanceValue.h"
#include "Param.h"

class ASTNode;
class Environment;

class FunctionValue : public std::enable_shared_from_this<FunctionValue> {
public:
    Value get(std::shared_ptr<InstanceValue>, std::shared_ptr<ClassValue>);
    QString toString() const;

    std::vector<Param> params;
    std::vector<std::shared_ptr<ASTNode>> body;
    std::shared_ptr<Environment> closure;
    QString name;
    QString ownerClassName;
};

#endif //CPPYTHON_FUNCTIONVALUE_H