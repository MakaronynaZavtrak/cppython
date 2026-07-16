//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_WHILENODE_H
#define CPPYTHON_WHILENODE_H

#include "../ASTNode.h"

/**
 * @class WhileNode
 * @brief Представляет узел цикла "while" в абстрактном синтаксическом дереве (AST).
 *
 * WhileNode моделирует цикл "while" с опциональными блоками "else" и отвечает
 * за выполнение тела цикла и опционального блока else, если условие цикла становится ложным.
 *
 * @details
 * WhileNode многократно вычисляет условие, пока оно истинно.
 * На каждой итерации выполняются инструкции из тела цикла. Особая обработка предусмотрена
 * для исключений `break` и `continue` внутри цикла:
 * - `ContinueException` приводит к пропуску оставшихся инструкций текущей итерации.
 * - `BreakException` немедленно завершает цикл.
 *
 * Если цикл завершается без прерывания с помощью "break", выполняется опциональный блок "else" (если он есть).
 * Возвращаемое значение метода `eval` — результат последнего выполненного выражения в теле цикла или теле else.
 */
class WhileNode final : public ASTNode {
public:
    WhileNode(std::shared_ptr<ASTNode> condition,
        std::vector<std::shared_ptr<ASTNode>> body,
        std::vector<std::shared_ptr<ASTNode>> elseBody);

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] bool shouldPrint() const override;

    [[nodiscard]] bool containsYield() const override;

    [[nodiscard]] Value evalResumable(EnvPtr env, ResumeContext& ctx) const override;

private:
    std::shared_ptr<ASTNode> condition;
    std::vector<std::shared_ptr<ASTNode>> body;
    std::vector<std::shared_ptr<ASTNode>> elseBody;
};
#endif //CPPYTHON_WHILENODE_H