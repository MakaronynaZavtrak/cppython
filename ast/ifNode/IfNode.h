//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_IFNODE_H
#define CPPYTHON_IFNODE_H

#include "../ASTNode.h"

/**
 * @class IfNode
 * @brief Представляет конструкцию условного ветвления в абстрактном синтаксическом дереве (AST).
 *
 * Класс IfNode моделирует оператор `if`, включая необязательные блоки `elif` и `else`.
 * Он оценивает условие и выполняет соответствующий блок, если условие истинно.
 * В случае множества ветвей `elif` они проверяются по порядку, пока одно из условий не окажется истинным.
 * Если ни одно из условий не выполнено, в случае наличия выполняется блок `else`.
 *
 * @details
 * Этот класс инкапсулирует логику и структуру оператора `if`. Он содержит:
 * - Узел `condition` с основным условием `if`.
 * - `body` — набор операторов для выполнения, если условие `if` истинно.
 * - Необязательную последовательность условий `elif` и соответствующих им блоков.
 * - Необязательный `elseBody` для выполнения, если ни одно из условий `if` или `elif` не оказалось истинным.
 *
 * Метод `eval` выполняет соответствующий блок, оценивая условия, и возвращает результат последнего выполненного оператора.
 * Метод `toString` восстанавливает текстовое представление структуры оператора `if`.
 */
class IfNode final : public ASTNode {
public:
    IfNode(std::shared_ptr<ASTNode> condition,
        std::vector<std::shared_ptr<ASTNode>> body,
        std::vector<std::pair<std::shared_ptr<ASTNode>, std::vector<std::shared_ptr<ASTNode>>>> elifs,
        std::vector<std::shared_ptr<ASTNode>> elseBody);

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] bool shouldPrint() const override;

    [[nodiscard]] bool containsYield() const override;

private:
    std::shared_ptr<ASTNode> condition;
    std::vector<std::shared_ptr<ASTNode>> body;
    std::vector<std::pair<std::shared_ptr<ASTNode>, std::vector<std::shared_ptr<ASTNode>>>> elifs;
    std::vector<std::shared_ptr<ASTNode>> elseBody;
};
#endif //CPPYTHON_IFNODE_H