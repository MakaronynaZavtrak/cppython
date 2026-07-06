//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_PASSNODE_H
#define CPPYTHON_PASSNODE_H

#include "../ASTNode.h"

class PassNode : public ASTNode {
public:

    [[nodiscard]] Value eval(std::shared_ptr<Environment>) const override;

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] bool shouldPrint() const override;
};
#endif //CPPYTHON_PASSNODE_H