//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_CONTINUENODE_H
#define CPPYTHON_CONTINUENODE_H

#include "../ASTNode.h"

/**
 * @class ContinueNode
 * @brief Представляет оператор "continue" в абстрактном синтаксическом дереве (AST) интерпретатора.
 *
 * ContinueNode — это специализированный тип ASTNode, соответствующий ключевому слову "continue" в исходном коде.
 * При вычислении он выбрасывает исключение ContinueException, чтобы сигнализировать о событии управления потоком,
 * пропускающем оставшиеся инструкции в текущей итерации цикла.
 *
 * @details
 * Этот узел используется для реализации поведения оператора "continue" в языках программирования.
 * Метод `eval` генерирует исключение ContinueException, которое перехватывается на соответствующем уровне для изменения
 * потока управления. Метод `toString` возвращает текстовое представление узла — строку "continue".
 */
class ContinueNode final : public ASTNode {

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;
};
#endif //CPPYTHON_CONTINUENODE_H