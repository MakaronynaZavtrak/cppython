//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_DELETENODE_H
#define CPPYTHON_DELETENODE_H

#include "../ASTNode.h"

//TODO: пока поддерживается только удаление содержимого в объектах
// чуть позже добавить поддержку:
// 1) del variable
// 2) del obj.attr
// 3) del a, b, c
class DeleteNode : public ASTNode {

public:

    std::shared_ptr<ASTNode> target;

    explicit DeleteNode(std::shared_ptr<ASTNode> target);

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] bool shouldPrint() const override;
};
#endif //CPPYTHON_DELETENODE_H