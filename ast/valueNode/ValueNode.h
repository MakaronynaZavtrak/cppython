//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_VALUENODE_H
#define CPPYTHON_VALUENODE_H

#include "../ASTNode.h"

/**
 * @class ValueNode
 * @brief Представляет узел, содержащий значение, в абстрактном синтаксическом дереве.
 *
 * ValueNode хранит конкретное значение, представленное экземпляром класса `Value`.
 * Этот узел используется для представления литеральных значений, таких как числа, строки, булевы значения и другие.
 *
 * @details
 * Узел предоставляет методы для вычисления (`eval`) и строкового представления (`toString`) своего значения.
 * Метод `eval` просто возвращает сохранённое значение, в то время как `toString` преобразует его в строку
 * на основе типа данных, содержащегося в `Value`. Класс является финальным и не может быть унаследован.
 */
class ValueNode final : public ASTNode {
public:
    Value value;

    explicit ValueNode(Value value);

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] Value eval(EnvPtr env) const override;
};
#endif //CPPYTHON_VALUENODE_H