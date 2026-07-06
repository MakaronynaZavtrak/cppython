//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_COMPARENODE_H
#define CPPYTHON_COMPARENODE_H

#include "../ASTNode.h"

/**
 * @class CompareNode
 * @brief Реализует узел для операций сравнения в абстрактном синтаксическом дереве (AST).
 *
 * CompareNode представляет операцию сравнения между левым операндом и одним или несколькими
 * правыми операндами с использованием операторов, таких как `<`, `<=`, `>`, `>=`, `==` или `!=`.
 * Он последовательно вычисляет сравнения и гарантирует, что все условия выполняются для сложных выражений.
 *
 * @details
 * Этот класс используется для вычисления выражений сравнения и получения логического результата. Он поддерживает
 * как строковые сравнения, так и числовые сравнения, в зависимости от типов вычисляемых операндов.
 * Если во время вычисления обнаруживается несоответствие (например, условие оператора не выполняется), вычисление
 * останавливается и возвращает `false`. В противном случае возвращается `true`, если все сравнения успешны. Логическая
 * цепочка сравнений позволяет естественным образом обрабатывать составные выражения (например, `a < b < c`).
 *
 * Метод `toString` создаёт текстовое представление выражения сравнения в формате AST.
 *
 * Класс не может быть далее унаследован и реализует поведение для узлов сравнения в AST.
 */
class CompareNode final : public ASTNode {
    std::shared_ptr<ASTNode> left;
    std::vector<QString> ops;
    std::vector<std::shared_ptr<ASTNode>> rights;

public:

    enum class Operation {
        Equal,
        NotEqual,
        Less,
        LessOrEqual,
        Greater,
        GreaterOrEqual,
        In,
        NotIn,
        Is,
        IsNot,
    };

    static Operation parseOperation(const QString &op);

    static bool compare(const Value& a, const Value& b, Operation op);

    CompareNode(std::shared_ptr<ASTNode> lhs,
                std::vector<QString> operators,
                std::vector<std::shared_ptr<ASTNode>> rgs);

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;
};
#endif //CPPYTHON_COMPARENODE_H