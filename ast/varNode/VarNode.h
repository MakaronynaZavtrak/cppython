//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_VARNODE_H
#define CPPYTHON_VARNODE_H

#include "../ASTNode.h"

/**
 * @class VarNode
 * @brief Представляет узел ссылки на переменную в абстрактном синтаксическом дереве (AST) интерпретатора.
 *
 * VarNode используется для представления переменных путем хранения их имен и предоставления механизмов
 * для определения их значений во время вычисления. Этот класс является финальным и не может быть далее унаследован.
 *
 * @details
 * Класс VarNode хранит имя переменной и предоставляет реализации методов
 * `eval` и `toString`. Во время вычисления он определяет значение переменной
 * путем поиска её в предоставленной среде выполнения. Также обеспечивает
 * представление имени переменной в виде строки.
 */
class VarNode final : public ASTNode {
public:
    QString name;

    explicit VarNode(QString name);

    [[nodiscard]] QString toString() const override;
    [[nodiscard]] Value eval(EnvPtr env) const override;
};
#endif //CPPYTHON_VARNODE_H