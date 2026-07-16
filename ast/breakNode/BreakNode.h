//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_BREAKNODE_H
#define CPPYTHON_BREAKNODE_H

#include "../ASTNode.h"

/**
 * @class BreakNode
 * @brief Представляет узел инструкции `break` в абстрактном синтаксическом дереве (AST).
 *
 * BreakNode — специализированный тип узла в AST, обозначающий выполнение
 * инструкции `break`, которая обычно используется для выхода из цикла или других
 * конструкций управления потоком. При вычислении он выбрасывает исключение BreakException,
 * чтобы сигнализировать о прерывании нормального потока выполнения.
 *
 * @details
 * Этот класс предназначен для появления в AST в тех местах исходного кода, где встречается
 * инструкция `break`. Его поведение при вычислении состоит исключительно в выбрасывании исключения,
 * а строковое представление — это слово "break". Этот узел нельзя наследовать или расширять,
 * так как он объявлен как final.
 */
class BreakNode final : public ASTNode {
public:
    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;
};
#endif //CPPYTHON_BREAKNODE_H