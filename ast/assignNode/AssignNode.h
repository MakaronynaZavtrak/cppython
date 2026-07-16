//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_ASSIGNNODE_H
#define CPPYTHON_ASSIGNNODE_H

#include "../ASTNode.h"

/**
 * @class AssignNode
 * @brief Представляет операцию присваивания в абстрактном синтаксическом дереве (AST) интерпретатора языка программирования.
 *
 * Класс AssignNode отвечает за присваивание значения переменной. Он вычисляет выражение справа от знака присваивания и
 * присваивает полученное значение переменной в переданной среде выполнения.
 *
 * @details
 * AssignNode — это конкретная реализация класса ASTNode, предназначенная для операций присваивания переменным.
 * При вычислении он вычисляет значение дочернего узла выражения, обновляет окружение, связывая имя переменной
 * с этим значением, и затем возвращает это значение. Строковое представление AssignNode записывается в стандартной
 * форме присваивания, объединяя имя переменной и выражение в виде "varName = valueExpr".
 */
class AssignNode final : public ASTNode {
public:
    AssignNode(QString varName, std::shared_ptr<ASTNode> valueExpr);

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] bool containsYield() const override;

    [[nodiscard]] bool shouldPrint() const override;

    [[nodiscard]] Value evalResumable(EnvPtr env, ResumeContext& ctx) const override;

private:
    QString varName;
    std::shared_ptr<ASTNode> valueExpr;
};
#endif //CPPYTHON_ASSIGNNODE_H